#include "chatvote.hpp"
#include "../../jrcommon/src/jrobshelpers.hpp"

#include <obs-module.h>
#include <obs.h>
#include <string>

#include <../obs-frontend-api/obs-frontend-api.h>
#include <../qt-wrappers.hpp>
#include <QObject>


#include <windows.h>
#include <shellapi.h>


#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>




ChatVote::ChatVote() {
	votePatternRe = QRegularExpression(R"(^(?:i\s)?(?:change\s)?(?:my\s)?vote[s]*[\s\:]+(?:for[\s\:])?(?:to[\s\:])?\s*?(?:\:)?\s*([^\(\)]*[^\s])\s*(?:\(.*\))?\s*$)" , QRegularExpression::CaseInsensitiveOption);
	votePatternRe.optimize();
	votePatternJesseRe = QRegularExpression(R"(^(?:jesse\s)?vote[s]*[\s\:]+(?:for[\s\:])?(?:to[\s\:])?\s*?(?:\:)?\s*([^\(\)]*[^\s])\s*(?:\(.*\))?\s*$)" , QRegularExpression::CaseInsensitiveOption);
	votePatternJesseRe.optimize();
}




void ChatVote::stop() {
	isOpenAndRunning = false;
}

bool ChatVote::reopen(bool flagClear) {
	if (voteListItemp) {
		isOpenAndRunning = true;
		if (flagClear) {
			userVotes.clear();
		}
		return true;
	}
	// we can't reopen without a voteListItemp set
	return false;
}


void ChatVote::clearAndRelease() {
	isOpenAndRunning = false;
	userVotes.clear();
	voteListItemp = NULL;
}

bool ChatVote::scanLineForVote(QString author, QString messageSimplePlaintext)  {
	// lowercase message first
	if (DefForceVoteStringsLowercase) {
		messageSimplePlaintext = messageSimplePlaintext.toLower();
	}
	// is it a vote regex?
	QRegularExpressionMatch match = votePatternRe.match(messageSimplePlaintext);
	if (!match.isValid() || !match.hasMatch()) {
		// no match, now check jesse vote message from users
		match = votePatternJesseRe.match(messageSimplePlaintext);
		if (!match.isValid() || !match.hasMatch()) {
			return false;
		}
		author = "Jesse (by proxy)";
	}
	// got a match
	QString voteItemStr = match.captured(1);
	addUserVote(author, voteItemStr);
	return true;	
}


void ChatVote::buildResults(QString &htmlResults, QString &plainResults, int &rowcount, int &maxrowwidth, bool &isOpen) {
	bool optionSortByVoteCount = true;

	// walk through user votes and add them to the votedItems list
	TStringStringMap votedItems;
	TStringIntMap votedItemCounts;
	foreach(auto item, userVotes) {
		QString userName = item.first;
		QString votedItem = item.second;
		if (votedItems.find(votedItem) == votedItems.end()) {
			// first entry
			votedItems[votedItem] = userName;
			votedItemCounts[votedItem] = 1;
		} else {
			// adding to it
			votedItems[votedItem] = votedItems[votedItem] + ", " + userName;
			++votedItemCounts[votedItem];
		}
	}


	// build vector for sorting
	std::vector< TStringIntPair > voteItemVector;
	for (auto itr = votedItemCounts.begin(); itr != votedItemCounts.end(); ++itr)
		voteItemVector.push_back(*itr);
	// sort by votes?
	if (optionSortByVoteCount) {
		sort(voteItemVector.begin(), voteItemVector.end(), [](TStringIntPair l, TStringIntPair r) {
			return l.second > r.second;
			});
	}

	// now build table
	QString instructions = R"(<span class="JrYtInstructions">(type: <span class="JrYtInstructionsType">vote YOUROPTION</span>)</span>)";
	isOpen = isOpenAndRunning;
	rowcount = 0;
	maxrowwidth = 20;
	QString outHtml;
	int totalVoteCount = 0;
	htmlResults = "";
	htmlResults += R"(<table class="JrYtTable">)";
	if (voteItemVector.size()>0) {
		htmlResults += R"(<thead>  <tr> <th>Option</th> <th class="jrColR">#</th> <th>Who?</th></tr> </thead>)";
	}
	htmlResults += "<tbody>";
	foreach(auto item, voteItemVector) {
		QString itemLabel = item.first;
		QString itemLabelDisplay = itemLabel;
		QString itemCount = QString::number(votedItemCounts[itemLabel]);
		QString itemUsers = votedItems[itemLabel];
		if (itemLabel == "") {
			itemLabelDisplay = "[WAITING]";
			if (itemUsers != "") {
				//itemUsers += ", and others.";
				itemUsers += ", ...";
			}
			itemCount = "&nbsp;";
		}
		QString row = R"(<tr> <td class="jrCol1">)" + itemLabelDisplay + R"(</td> <td class="jrColR jrColNw">)" + itemCount + R"(</td> <td class="jrCol">)" + itemUsers + "</td> </tr>\n";
		if (itemLabel != "") {
			totalVoteCount += votedItemCounts[itemLabel];
		}
		htmlResults += row;
		//
		++rowcount;
		int thisWidth = itemLabel.length() + itemUsers.length() + 5;
		if (thisWidth > maxrowwidth) {
			maxrowwidth = thisWidth;
		}
	}
	if (rowcount == 0) {
		QString row;
		if (isOpen) {
			row = R"(<tr> <td colspan="3" class="jrColM">waiting for votes..</td> </tr>)";
		}
		else {
			row = R"(<tr> <td colspan="3" class="jrColM">no entries.</td> </tr>)";
		}
		htmlResults += row;
		++rowcount;
	}
	// header and footer
	rowcount += 2;
	htmlResults += "</tbody>";
	QString row = R"(<tfoot><tr> <td class="jrCol">Participants:</td> <td class="jrColR jrColNw">)" + QString::number(totalVoteCount) + R"(</td> <td class="jrColR">)" + instructions + "</td> </tr></tfoot>\n";
	htmlResults += row;
	htmlResults += "</table>";

	plainResults = QString::number(totalVoteCount) + " votes";

}




void ChatVote::addUserVote(const QString &author, const QString &choiceStr)  {
	userVotes[author] = choiceStr;
}






//---------------------------------------------------------------------------
void ChatVote::startNew(QListWidgetItem* itemp) {
	// create a new listitem to hold results of vote

	// start by remembering pointer to listitem we work with
	setVoteListItemp(itemp);

	// ok we are up and running
	isOpenAndRunning = true;
	userVotes.clear();
}
//---------------------------------------------------------------------------
