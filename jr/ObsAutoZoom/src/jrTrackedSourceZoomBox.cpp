//---------------------------------------------------------------------------
#include "jrSourceTracker.h"
#include "jrPlugin.h"
//
#include "../../jrcommon/src/jrhelpers.hpp"
#include "../../jrcommon/src/jrobshelpers.hpp"
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
#define DefTestDontHuntOnOnlyOneMissingMarker true
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void TrackedSource::updateZoomCropBoxFromCurrentCandidate(bool debugPreviewOnly) {
	// ok we have a CANDIDATE box markerx1,markery1,markerx2,markery2 and/or a planned next place to go
	// now we want to change our actual zoomCrop box towards this


	if (debugPreviewOnly) {
		// this should not be called for debug only mode
		return;
	}

	// pointers
	JrRegionDetector* rd = &regionDetector;
	JrPlugin* plugin = getPluginp();

	// shorthand vars we will set

	// actually decided and planned goal targets to move towards
	int goalx1, goaly1, goalx2, goaly2;
	int targetx1, targety1, targetx2, targety2;

	bool oneMarkerIsOccluded = false;
	bool isTargetMarkerless = false;
	bool bothMarkersValid = true;

	// stickiness DEFAULT counter
	int changeMomentumCountTrigger = plugin->computedMomentumCounterTargetNormal;

	// ok get current target location (either of found box, or of whole screen if no markers found)
	if (areMarkersBothVisibleOrOneOccluded()) {
		// we have a new box, set current instant targets to it
		//mydebug("Using marker coords %d,%d - %d,%d.", markerx1, markery1, markerx2, markery2);
		targetx1 = markerx1;
		targety1 = markery1;
		targetx2 = markerx2;
		targety2 = markery2;
		//
		if (markerBoxIsOccluded) {
			 // we have valid positions BUT one is occluded so we done trust it
			// setting this makes us hunt for a wider view once we settle
			oneMarkerIsOccluded = true;
			bothMarkersValid = false;
			// dont interrupt any hunting
		} else {
			// we found a good box again, so cancel any hunting for a wider view..
			// it's not clear we really need to do this or if caller will stop hunting or if this will prevent triggering of end of hunt, but since we found markers, it shouldn't matter, we will stay here.
			sourceTrackerp->abortHuntingIfPullingWide();
		}
	} else {
		// we have not found a valid tracking box
		isTargetMarkerless = true;
		bothMarkersValid = false;

		//mydebug("Using lasttarget coords %d,%d - %d,%d.", lasttargetx1, lasttargety1, lasttargetx2, lasttargety2);

		// in this case we now leave targets alone -- either leave them where they were, or leave them at what the external function set (markerless coords)
		// the only thing we want to not do is use this target as a STABLE accumulating target
		// maybe the easier thing to do when there are no targets is set target to sticky goal
		if (true) {
			// when we do this we dont really have to worry about any of the other checks below
			targetx1 = (int)stickygoalx1;
			targety1 = (int)stickygoaly1;
			targetx2 = (int)stickygoalx2;
			targety2 = (int)stickygoaly2;
		}
		else {
			targetx1 = (int)lasttargetx1;
			targety1 = (int)lasttargety1;
			targetx2 = (int)lasttargetx2;
			targety2 = (int)lasttargety2;
		}

		// set stickiness counter to be higher so that we wait longer before moving 
		changeMomentumCountTrigger = plugin->computedMomentumCounterTargetMissingMarkers;
		// ATTN: new -- rather than computing markerless targets, etc. which we used to do, now we let external functions decide when and what to set our targets to in this markerless case
	}

	// if no current box, then instantly initialize to the whole screen as our starting (current) point
	if (!lookingBoxReady) {
		// we need to initialize where we are looking; use our markerless default (full screen normally)
		// ATTN: NEW - we may want to switch to a new source entirely, which is outside our purview of this function
		// in that case, what should we do
		lookingx1 = 0;
		lookingy1 = 0;
		lookingx2 = sourceWidth;
		lookingy2 = sourceHeight;
	}

	// should we immediately move towards our chosen target, or do we want some stickiness at current location or previous target to avoid JrPlugin::jitters and momentary missing markers?
	if (!stickyBoxReady) {
		// we dont have a prior target, so we use current location as our initial default prior "sticky" location
		stickygoalx1 = (float)lookingx1;
		stickygoaly1 = (float)lookingy1;
		stickygoalx2 = (float)lookingx2;
		stickygoaly2 = (float)lookingy2;
		// ready from here on out
		stickyBoxReady = true;
	}


	// at this point we have our current view focus (lookingx1...) and our newly found tracking box (markerx1) and our PRIOR box location (stickygoalx1)
	// now decide whether to move to the new box (tx values or stick with next)
	bool switchToNewTarget = false;
	bool doDriftSlowlyToTarget = false;
	bool newTargetInMotion = true;
	bool targetIsStable = false;
	float distanceThresholdTriggersMove;
	bool markersAreMoving = true;

	// decide how much distance away triggers a need to change to a "new" target
	if (changeTargetMomentumCounter >= changeMomentumCountTrigger) {
		// if we have exceeded stickiness and are now moving, then we insist on getting CLOSER to target than normal
		distanceThresholdTriggersMove = plugin->computedChangeMomentumDistanceThresholdMoving * sourceDistMult;
	} else {
		// once we are stabilized we require more deviation before we break off and start moving again
		distanceThresholdTriggersMove = plugin->computedChangeMomentumDistanceThresholdStabilized * sourceDistMult;
	}

	// figure out if the target has been steady motionless for a while
	if (!lastTargetBoxReady) {
		// reset to default
		targetStableCount = 0;
	} else {
		// ok we can compare the new change in target location and accumulate it, so we we figure out when target is in motion
		float thisTargetDelta = (float)jrRectDist(targetx1, targety1, targetx2, targety2, lasttargetx1, lasttargety1, lasttargetx2, lasttargety2);
		if (thisTargetDelta < plugin->computedThresholdTargetStableDistance * sourceDistMult) {
			// target has stayed still, increase stable count
			++targetStableCount;
			if (targetStableCount > plugin->computedThresholdTargetStableCount) {
				// stable for long enough to consider it NOT in motion
				newTargetInMotion = false;
			}
		} else {
			// target moved too much, reset counter
			targetStableCount = 0;
		}
	}
	//
	// remember last target locations so we can keep track if target is changing position
	lasttargetx1 = targetx1;
	lasttargety1 = targety1;
	lasttargetx2 = targetx2;
	lasttargety2 = targety2;
	lastTargetBoxReady = true;

	// compute distance between new target from current position
	float newTargetDeltaLooking = (float)jrRectDist(targetx1, targety1, targetx2, targety2, lookingx1, lookingy1, lookingx2, lookingy2);
	// and distance between new target and our last sticky goal
	float newTargetDeltaSticky = (float)jrRectDist(targetx1, targety1, targetx2, targety2, stickygoalx1, stickygoaly1, stickygoalx2, stickygoaly2);

	//mydebug("Delta newTargetDeltaLooking = %f  and newTargetDeltaSticky = %d and newTargetInMotion = %d.", newTargetDeltaLooking, newTargetDeltaSticky, (int)newTargetInMotion);


	//
	if (newTargetInMotion) {
		// when new target is clearly in motion, we avoid making any changes to where we are looking
		// instead we wait for target to stabilize then we move
		// so we just leave stickygoal whatever it was
		//mydebug("New target is in motion so not setting averageCloseToTarget.");
		changeTargetMomentumCounter = 0;
	}
	else if (false && isTargetMarkerless) {
		// in this case we dont want to switch or move to target, just drop down and check for markers in wider view
		targetIsStable = true;
		changeTargetMomentumCounter = 0;
	} else if (newTargetDeltaLooking > distanceThresholdTriggersMove) {
		// target is not in motion and is far enough from where we are *looking*
		// now decide whether it's close to our current goal where we are already moving to
		//mydebug("Target is fare enough from where we are looking %f > %f.",newTargetDeltaBoxAll, distanceThresholdTriggersMove);
		if (newTargetDeltaSticky < distanceThresholdTriggersMove) {
			// it's close enough to current sticky goal, so let it drift but dont pick new target, and dont reset counter so we will be fast to react if it does move further waway
			// this also helps us more clearly know when we are choosing a dramatically new target and not set switchToNewTarget when doing minor tweaks
			// and note that moving to this does not requiring building momentum and overcoming stickiness
			//mydebug("avergageToCloseTarget setting true.");
			doDriftSlowlyToTarget = true;
		} else {
			// new target is far enough way from our current sticky goal, lets build some momentum to switch to it
			//mydebug("new target is far enough way from sticky goal: %f not < %f.",newTargetDeltaBoxAll, distanceThresholdTriggersMove);
			++changeTargetMomentumCounter;
			if (changeTargetMomentumCounter >= changeMomentumCountTrigger) {
				// ok we have exceeded stickiness, we are ready to switch to it as our new sticky goal going forward
				// note that if operators moves markers slowly, we can transition between far away places without ever triggering switchToNewTarget
				switchToNewTarget = true;
				// 
				// should we reset momentum counter? NO we want to quickly move to it once we start moving; only when we stop and stabilize do we reset
				// instead we clamp it to current value so that it wont grow so high as to trigger on missing markers
				// but note that because we don't reset this, we will rapidly be switching to new target over and over again once this triggers
				// ATTN: do we want to check if the new target is different from current target and not keep doing this if so -- so we only trigger on FIRST change of target
				changeTargetMomentumCounter = changeMomentumCountTrigger;
			}
		}
	} else {
		// target is stable and where we are very near, so reset momentum to make sticky the next move
		targetIsStable = true;
		changeTargetMomentumCounter = 0;
		//mydebug("target is stable comparing %f >? %f.",newTargetDeltaBoxAll, plugin->computedChangeMomentumDistanceThresholdDontDrift);
		// but let's drift slowly to new target averaging out
		if (newTargetDeltaLooking> plugin->computedChangeMomentumDistanceThresholdDontDrift * sourceDistMult) {
			//mydebug("Setting doDriftSlowlyToTarget true.");
			doDriftSlowlyToTarget = true;
		} else {
			// it's close enough that we will not even TRY to adjust; this will keep it from jittering due to camera inperfections
		}
	}


	// ok now we have to decide if we want to update STICKY goal, or leave it where it is
	// so our GOAL during every cycle is to move towards sticky position
	// and the question is just when to update sticky

	// immediately switch over to move to newly found target box?

	if (switchToNewTarget) {
		// make our new target the newly found box
		//mydebug("Switch to new target active.");
		stickygoalx1 = (float)targetx1;
		stickygoaly1 = (float)targety1;
		stickygoalx2 = (float)targetx2;
		stickygoaly2 = (float)targety2;
		// we set this to 1 so that it will begin accumulating settledLookingStreakCounter below when it stabilizes its location
		settledLookingStreakCounter = 1;
	}
	else if (doDriftSlowlyToTarget) {
		//mydebug("In doDriftSlowlyToTarget 1.");
		// we consider ourselves close enough to the new target that we arent going to do anything, but we can drift average out towards it
		// the averageing should stop jitter but the drifting should keep us from locking on a point 1 pixel away that is wrong
		float computedAverageSmoothingRateToCloseTarget = plugin->computedAverageSmoothingRateToCloseTarget;
		stickygoalx1 = (computedAverageSmoothingRateToCloseTarget * (float)stickygoalx1)  + ((1.0f - computedAverageSmoothingRateToCloseTarget) * (float)targetx1);
		stickygoaly1 = (computedAverageSmoothingRateToCloseTarget * (float)stickygoaly1)  + ((1.0f - computedAverageSmoothingRateToCloseTarget) * (float)targety1);
		stickygoalx2 = (computedAverageSmoothingRateToCloseTarget * (float)stickygoalx2)  + ((1.0f - computedAverageSmoothingRateToCloseTarget) * (float)targetx2);
		stickygoaly2 = (computedAverageSmoothingRateToCloseTarget * (float)stickygoaly2)  + ((1.0f - computedAverageSmoothingRateToCloseTarget) * (float)targety2);
		// this is weird but letting prior vals be floats causes painful flickering of size when locations flicker
		if (plugin->computedIntegerizeStableAveragingLocation) {
			stickygoalx1 = (float)stickygoalx1;
			stickygoaly1 = (float)stickygoaly1;
			stickygoalx2 = (float)stickygoalx2;
			stickygoaly2 = (float)stickygoaly2;
		}
		// we ALWAYS increase settledLookingStreakCounter in this case where we are TWEAKING the position
		++settledLookingStreakCounter;
	} else {
		// remember that we can get here, not switch to new target, even though target is far from sticky target.. we are just waiting for momentum to build before we switch to new target
		// save this flag so we can check for it later
		markersAreMoving = false;
	}

	//
	// remember good sticky marker goals
	if (bothMarkersValid && areMakersExamined() && (switchToNewTarget || doDriftSlowlyToTarget)) {
		sourceTrackerp->saveGoodMarkerPosition((int)stickygoalx1, (int)stickygoaly1, (int)stickygoalx2, (int)stickygoaly2, index);
		}


	//mydebug("sticky position is %f,%f-%f,%f.", stickygoalx1, stickygoaly1, stickygoalx2, stickygoaly2);
	//mydebug("target position is %d,%d-%d,%d.", targetx1, targety1, targetx2, targety2);


	// ok now we want to know if we are DONE moving our looking, because we are looking at sticky and sticky is at target
	bool isLookingPositionSettled = false;
	// dist1 is from sticky to looking
	float dist1 = (float)jrRectDist(stickygoalx1, stickygoaly1, stickygoalx2, stickygoaly2, lookingx1, lookingy1, lookingx2, lookingy2);
	// dist2 is from target (next goal) to sticky (current goal); note that if we are markerless then we dont trust our target locations?
	//float dist2 = isTargetMarkerless ? 0.0 : jrRectDist(stickygoalx1, stickygoaly1, stickygoalx2, stickygoaly2, targetx1, targety1, targetx2, targety2);
	float dist2 = (float)jrRectDist(stickygoalx1, stickygoaly1, stickygoalx2, stickygoaly2, targetx1, targety1, targetx2, targety2);
	float dist3 = (float)max(dist1, dist2);
	//
	float settledLookingThresh = (float)max(plugin->computedChangeMomentumDistanceThresholdDontDrift, plugin->computedThresholdTargetStableDistance) * sourceDistMult;
	if (targetIsStable && dist3 <= settledLookingThresh) {
		isLookingPositionSettled = true;
	} else {
		isLookingPositionSettled = false;
	}

	// ATTN: 12/8/22 attempt to get one shot no target to work well
	if (isTargetMarkerless && !plugin->didOneShotFindValidTarget()) {
		isLookingPositionSettled = true;
	}

	if (plugin->oneShotEngaged) {
		//mydebug("ATTN:oneshot - in updateZoomCropBoxFromCurrentCandidate with isLookingPositionSettled=%d  isTargetMarkerless=%d  isOneShotEngaged() = %d.",(int)isLookingPositionSettled, (int)isTargetMarkerless, (int)plugin->isOneShotEngaged());
	}


	//mydebug("[ Index %d ]  looking %d,%d - %d,%d   with target %d,%d - %d,%d  and sticky %f,%f - %f,%f  with dist1 = %f and dist2 = %f and dist3 = %f and settledLookingThresh = %f and lookingsettled = %d and targetstable=%d.", index, lookingx1, lookingy1, lookingx2, lookingy2, targetx1, targety1, targetx2, targety2, stickygoalx1, stickygoaly1, stickygoalx2, stickygoaly2, dist1, dist2, dist3, settledLookingThresh, (int)isLookingPositionSettled, (int)targetIsStable);

	if (isLookingPositionSettled && (plugin->opt_autoTrack || plugin->isOneShotEngaged())) {
		// ok we are settled on where we are looking and our goal, so we are going to consider doing a hunt
		// this only happens if we are auto updating or in a oneshot

		// a little unusual but if we dont change at all since we checked, then dont increase this; only if we have started a tweak
		// this is to leave this at 0 after we use it to check (ie it will only ever be 0 after we have reset it after checking it)
		if (settledLookingStreakCounter > 0) {
			++settledLookingStreakCounter;
		}

		// ok determine if the markers have moved far enough to consider this worth our normal fast hunt
		float huntdist = 0.0;
		// attn: only use huntdist if BOTH markers valid
		if (bothMarkersValid) {
			huntdist = (float)jrRectDist(lasthuntx1, lasthunty1, lasthuntx2, lasthunty2, markerx1, markery1, markerx2, markery2);
		}


		bool markersMovedEnoughForRehunt = huntdist > plugin->computedThresholdTargetStableDistance * sourceDistMult;

		//mydebug("ATTN:oneshot - IN isLookingPositionSettled.huntdist = % f and moveEnoughForFastHunt = % d.",huntdist, (int)markersMovedEnoughForRehunt);

		// travelingToNewTarget is set each time we choose a new target to go to, and only reset here when we arrive somewhere
		if (markersMovedEnoughForRehunt || (bothMarkersValid && plugin->isOneShotEngagedAndFirstStage()) ) {
			// we just arrived at new target we've been traveling to; we could so special stuff on this once-per-new-target arrival; this might be good time to hunt for better views
			//mydebug("---------- [YES] Markers have moved enough to trigger fast hunt; bothmarkersfound= %d.", (int)bothMarkersValid);
			if (bothMarkersValid || (DefTestDontHuntOnOnlyOneMissingMarker && oneMarkerIsOccluded)) {
				// we have just arrived at a good marker target, so check for zoomed in good view
				//mydebug("TESTING bothMarkersValid initiateHunt new target push in.");
				sourceTrackerp->initiateHunt(EnumHuntType_NewTargetPushIn, false);
			} else {
				// stabilized on markerless, do a zoom out check
				// ATTN: im not sure this really should happen much, since in this markerless case we probably dont have a target
				//mydebug("TESTING NOTbothMarkersValid initiateHunt lost target pull out with markersMovedEnoughForRehunt = %d.", (int) markersMovedEnoughForRehunt);
				sourceTrackerp->initiateHunt(EnumHuntType_TargetLostPullOut, false);
			}
			// reset these since we have just hunted
			settledLookingStreakCounter = 1;
			markerlessStreakCounter = 0;
			markerlessStreakCycleCounter = 0;
			// hunt clear
			updateAfterHuntCheck();
			// oneshot stage advance or disable
			plugin->updateOneShotStatus(isLookingPositionSettled, bothMarkersValid);
		} else {
			// markers havent moved enough to trigger a rehunt, but we still check occasionally
			//mydebug("Markers have NOT moved enough to retrigger fast hunt.");
			// ATTN: if both markers aren't valid than we don't test for push in.. is this what we want or might we try for push in even if WE can't see both markers...
			if (bothMarkersValid || (DefTestDontHuntOnOnlyOneMissingMarker && oneMarkerIsOccluded)) {
				// both good markers found.. but we will check again occasionally just to make sure
				//mydebug("ATTN: both markers valid; TESTING settledLookingStreakCounter = %d.", settledLookingStreakCounter);
				if (settledLookingStreakCounter > DefSettledLookingPositionRecheckLimit || plugin->isOneShotEngagedAndFirstStage()) {
					// markers haven't moved far enough since out last hunt check, BUT enough time has passed that we will check again to make sure
					if (false) {
						// ATTN: this seems to be problematic, it seems we can fail here and then never check again; so instead we will try doing a normal pull out every time we reach threshold
						// but note that for now we set settledLookingStreakCounter to 0 which will BLOCK reaccumulating on this until the markers move somewhat
						settledLookingStreakCounter = 0;
						//mydebug("TESTING initiateHunt EnumHuntType_LongTimeStable.");
						sourceTrackerp->initiateHunt(EnumHuntType_LongTimeStable, false);
					} else {
						// set settledLookingStreakCounter to 1 so we will keep repeating this settledLookingStreakCounter and keep rechecking; if we set it to 1 then we will only do it ONCE per settling
						settledLookingStreakCounter = 1;
						//mydebug("ATTN: TESTING initiateHunt from slowchangestreak EnumHuntType_NewTargetPushIn.");
						sourceTrackerp->initiateHunt(EnumHuntType_NewTargetPushIn, false);
					}
				updateAfterHuntCheck();
				// oneshot stage advance or disable
				plugin->updateOneShotStatus(isLookingPositionSettled, bothMarkersValid);
				}
				else {
					// settled count isnt high enough, do nothing
					//mydebug("!targetless but settled look, with settledLookingStreakCounter == %d.",settledLookingStreakCounter);
				}
				// reset since we have two good markers
				markerlessStreakCounter = 0;
				markerlessStreakCycleCounter = 0;
			} else {
				// markers not found (or only one); has long enough of this gone by to check again? this will never stop and check EVERY opt_zcMissingMarkerTimeout cycles
				// one problem here is that we are looking both for wider markers, AND to catch a markerless situation, but we might like to delay the latter more
				//mydebug("ATTN:oneshot MARKERS NOT FOUND on index %d -- onemark = %d TESTING markerlessStreakCounter = %d vs %d.", index, (int)oneMarkerIsOccluded, markerlessStreakCounter,plugin->opt_zcMissingMarkerTimeout);
				++markerlessStreakCounter;
				if (!oneMarkerIsOccluded) {
					// increase this counter more if NO markers seen, so we could be jumping +2 per time
					++markerlessStreakCounter;
				}
				if (plugin->isOneShotEngaged()) {
					// 12/8/22 attempt to go earlier to missing target
					markerlessStreakCounter += 2;
				}
				//mydebug("ATTN:oneshot - both markers not found,  markerlessStreakCounters : %d.", markerlessStreakCounter);
				if (plugin->isOneShotEngaged() && plugin->didOneShotFindValidTarget()) {
					// we found a good oneshot target earlier, so we do NOT allow ourselves to pull out if they are removed by user
					// this ensures that we essentially stop looking for markers after a oneshot finds something
					plugin->updateOneShotStatus(isLookingPositionSettled, bothMarkersValid);
					//mydebug("ATTN:oneshot earlier found target so avoiding going to markerless.");
//				} else if (markerlessStreakCounter > plugin->opt_zcMissingMarkerTimeout || plugin->isOneShotEngagedAndFirstStage()) {
				} else if (markerlessStreakCounter > plugin->opt_zcMissingMarkerTimeout) {
					// we've been markerless on this source for long enough, lets check again; this will happen regularly
					// we will switch to the markerless view only if its not the case that we have an occluded marker and we have done enough cycles of checking this for a wider marker first

					bool shouldConcludeNoMarkersAnywhereAndSwitch = (!oneMarkerIsOccluded && (markerlessStreakCycleCounter >= DefMarkerlessStreakCyclesBeforeSwitch)) || plugin->isOneShotEngaged();

					//mydebug("ATTN:oneshot TESTING initiateHunt markerlessStreakCounter : %d shouldConclude = %d.", markerlessStreakCounter, (int)shouldConcludeNoMarkersAnywhereAndSwitch);

					sourceTrackerp->initiateHunt(EnumHuntType_LongTimeMarkerless, shouldConcludeNoMarkersAnywhereAndSwitch);

					// hunt clear
					updateAfterHuntCheck();
					// oneshot stage advance or disable
					plugin->updateOneShotStatus(isLookingPositionSettled, bothMarkersValid);
					// reset normal markerlessStreakCounter
					markerlessStreakCounter = 0;
					//  now every time a streakcounter passes threshold we increase our markerlessStreakCycleCounter counter, and when that passes thereshold we will be willing to jump to markerless conclusion
					if (shouldConcludeNoMarkersAnywhereAndSwitch) {
						markerlessStreakCycleCounter = 0;
					}
					else if (!oneMarkerIsOccluded) {
						++markerlessStreakCycleCounter;
					}
					else {
						// dont count this situation where we have an occluded marker -- it doesnt make us speed up check for no markers switch
					}
				}
			}
		}
	} else {
		// looking is moving.. so reset these
		resetMarkerLessCounters();
		plugin->updateOneShotStatus(isLookingPositionSettled, bothMarkersValid);
	}

	// ok now we have our chosen current TARGET updated in stickygoalx1
	// and we will move towards it in some gradual or instant way
	goalx1 = (int)stickygoalx1;
	goaly1 = (int)stickygoaly1;
	goalx2 = (int)stickygoalx2;
	goaly2 = (int)stickygoaly2;


	// some options at this point are:
	// 1. instantly switch to the new box
	// 2. smoothly move to the new box

	// move towards new box

	if (plugin->opt_zcBoxMoveSpeed == 0 || plugin->opt_zcEasing == 0) {
		// instant jump what we are looking at to new target
		lookingx1 = goalx1;
		lookingy1 = goaly1;
		lookingx2 = goalx2;
		lookingy2 = goaly2;
	}
	else if (plugin->opt_zcEasing == 1 || true) {
		// gradually move towards new box

		// we are go to morph/move MIDPOINT to MIDPOINT and dimensions to dimensions
		// delta(change) measurements
		float xmid = (float)(goalx1 + goalx2) / 2.0f;
		float ymid = (float)(goaly1 + goaly2) / 2.0f;
		float bxmid = (float)(lookingx1 + lookingx2) / 2.0f;
		float bymid = (float)(lookingy1 + lookingy2) / 2.0f;
		float deltaXMid = (xmid - bxmid);
		float deltaYMid = (ymid - bymid);
		float bwidth = (float)(lookingx2 - lookingx1);
		float bheight = (float)(lookingy2 - lookingy1);
		float deltaWidth = (float)(abs(goalx2 - goalx1) - fabs(bwidth));
		float deltaHeight = (float)(abs(goaly2 - goaly1) - fabs(bheight));
		float largestDelta = (float)jrmax4(fabs(deltaXMid), fabs(deltaYMid), fabs(deltaWidth), fabs(deltaHeight));

		float baseMoveSpeed = (float)(plugin->opt_zcBoxMoveSpeed) * sourceDistMult;
		// modify speed upward based on distance to cover
		float moveSpeedMult = 50.0f;
		float maxMoveSpeed = baseMoveSpeed * (1.0f + moveSpeedMult * (largestDelta / (float)max(sourceWidth, sourceHeight)));
		float safeDelta = (float)min(largestDelta, maxMoveSpeed);
		float percentMove = (float)min(safeDelta / largestDelta, 1.0);
		//mydebug("Debugging boxmove deltaWidth = %f  deltaHeight = %f  deltaXMid = %f deltaYmid = %f   largestDelta = %f  safeDelta = %f   percentMove = %f.",deltaWidth, deltaHeight, deltaXMid, deltaYMid, largestDelta, safeDelta, percentMove);
		//
		// adjust b midpoints and dimensions
		bxmid += deltaXMid * percentMove;
		bymid += deltaYMid * percentMove;
		bwidth += deltaWidth * percentMove;
		bheight += deltaHeight * percentMove;

		// now set goalx1, etc. baded on width and new dimensions
		lookingx1 = (int)(bxmid - (bwidth / 2.0f));
		lookingy1 = (int)(bymid - (bheight / 2.0f));
		lookingx2 = (int)(lookingx1 + bwidth);
		lookingy2 = (int)(lookingy1 + bheight);

	}

	// note that we have a cropable box (this does not go false just because we dont find a new one -- it MIGHT stay valid)
	lookingBoxReady = true;
}


void TrackedSource::resetMarkerLessCounters() {
	settledLookingStreakCounter = 1;
	markerlessStreakCounter = 0;
	markerlessStreakCycleCounter = 0;
}
//---------------------------------------------------------------------------

























