# blink(1) usb device
from blink1.blink1 import Blink1
# pytchat
import pytchat
# global hotkeys
import global_hotkeys as ghotkeys

# utils
import time
import argparse
import signal
import re

import signal

import os
import sys
#import win32api


# important to not have pipe errors regarding output format
sys.stdout.reconfigure(encoding='utf-8')


# global app and version info
appName = "ytChatMon"
appVersion = "2.2 (3/6/23)"
appDescription = "YouTube Live Chat Monitor"
#appPrintPrefix = "] " + appName + " v"+ appVersion
appPrintPrefix = "] "




# other options
defSleepTimeBetweenChatGets = 5  # note that a value as small as 5 can get temporarily blocked by youtube, but script will reconnect and continue (5 works but kicks)
defSleepTimeBetweenChatProcess = 0.1  # how many seconds to wait between parsing each chat message
defSleepTimeBetweenReconnect = 30  # how many seconds to wait before trying to reconnect on chat disconnect (30 is safe)
defTooLongOnTime = 180  # shuts off alert light after this many seconds (normally you would do it with button)
optionReconnect = True  # reconnect after chat connection lost?
optionBrief = False
optionFileBrief = True
optionBlink = True
blinkColorAlert = "#FF0000" #"red"
blinkColorNetworkIssue = "#00003F" #"blue"
blinkColorStartupTest =  "#00FF00" #"green"
optionForceReplay = False # disabled does not work
optionUseContinuationOnReconnect = True
optionUseContinationEvenIfLive = False  # unused for now
optionPythonNewerPytchatKludge = True
optionRegexPatternAlert = r"jesse.*check|check.*jesse"
optionRegexPatternCancel = r"cancel alert"
optionReconnectAttemptLimit = 3;

# regex patterns to trigger alerts and cancel them
patternAlert = False
patternCancel = False





# global vars
wantsQuit = False
lastBlinkOnTime = 0
youtubeVideoId = False
outFilePath = ""
# global objects
blink = False
chat = False
ytChatStreamContinuation = False
outFile = False
kthread = False
reconnectAttempts = 0






import threading

class KeyboardThread(threading.Thread):
    def __init__(self, input_cbk = None, name='keyboard-input-thread'):
        self.input_cbk = input_cbk
        super(KeyboardThread, self).__init__(name=name)
        self.start()
    def run(self):
        global wantsQuit
        try:
            while not wantsQuit:
                self.input_cbk(input()) #waits to get input + Return
        except:
            setWantsQuit()

def keybpardThreadCallback(inp):
    #evaluate the keyboard input
    for c in inp:
        #if (c==b'\x03' or c==b'\x04' or c=='\x04' or c=='\x03' or c=="\x04" or c=="\x03"):
        if (c=="\x04" or c=="\x03"):
            myprint("Aborting processing (ctrl+C / ctrl+D).")
            setWantsQuit()
        else:
            pass


# generic
def setup():
    global youtubeVideoId
    #
    parseCommandlineArgs()
    setupRegex();
    #
    setupOutputFile()
    setupBlink()
    setupHotkeys()
    setupPytchat(youtubeVideoId, False)
    # attempt to catch kill signals
    signal.signal(signal.SIGINT, lambda a, b: abortApplication())
    #signal.signal(signal.SIGTERM, lambda a, b: abortApplication())
    #win32api.SetConsoleCtrlHandler(callbackRecieveSignal, True)
    global kthread
    kthread = KeyboardThread(keybpardThreadCallback)

def shutdown():
    shutdownPytchat()
    shutdownHotkeys()
    clearBlink()
    shutdownBlink()
    shutdownOutputFile()
    setWantsQuit()
    myprint("Exiting.")
    exit();
    #sys.exit()
    

def doLoopTillQuit():
    global wantsQuit
    while (not wantsQuit):
        time.sleep(0.1)

def myprint(*args, **kwargs):
    global appPrintPrefix
    print( appPrintPrefix + " ".join(map(str,args)), **kwargs)

def parseCommandlineArgs():
    global appName, appDescription
    global optionRegexPatternAlert, optionRegexPatternCancel

    parser = argparse.ArgumentParser(prog = appName, description = appDescription)
    parser.add_argument('-v', '--videoid', required=True)
    parser.add_argument('-o', '--outfile', default = False)
    parser.add_argument('-b', '--brief', default = False, action='store_true')
    parser.add_argument('-n', '--noblink', default = False, action='store_true')
    parser.add_argument('-l', '--live', default = False, action='store_true')
    parser.add_argument('-a', '--regexalert', default = optionRegexPatternAlert)
    parser.add_argument('-c', '--regexcancel', default = optionRegexPatternCancel)
    args = parser.parse_args()
    # store video id global
    global youtubeVideoId, outFilePath, optionBrief, optionBlink, optionUseContinuationOnReconnect
    youtubeVideoId = args.videoid
    outFilePath = args.outfile
    optionBrief = args.brief
    optionBlink = not args.noblink
    optionUseContinuationOnReconnect = not args.live
    optionRegexPatternAlert = args.regexalert
    optionRegexPatternCancel = args.regexcancel

def setupRegex():
    # regex compile
    global optionRegexPatternAlert, optionRegexPatternCancel, patternAlert, patternCancel
    patternAlert = re.compile(optionRegexPatternAlert, flags=re.I)
    patternCancel = re.compile(optionRegexPatternCancel, flags=re.I)


def abortApplication():
    global wantsQuit
    wantsQuit = True
    myprint("Aborting processing (signit).")

def loopSleepWantsQuitStop(secs):
    global wantsQuit
    curtime = 0
    perSleep = 0.25
    while (curtime < secs):
        if (wantsQuit):
            break
        curtime += perSleep
        time.sleep(perSleep)
    pass

def setWantsQuit():
    global wantsQuit
    wantsQuit = True

def callbackRecieveSignal(sigVal):
    setWantsQuit()



# blink helpers
def setupBlink():
    if (not optionBlink):
        return
    #myprint("Initializing blink device.")
    global blink
    blink = Blink1()
    # test it
    testShowBlink()
    # clear it
    #clearBlink()

def shutdownBlink():
    global blink
    if (not blink):
        return
    blink.close()
    blink = False
    
def updateLastBlinkOnTime():
    global lastBlinkOnTime
    lastBlinkOnTime = time.time()
def clearLastBlinkOnTime():
    global lastBlinkOnTime
    lastBlinkOnTime = 0
def turnOffBlinkIfOnTooLong():
    global lastBlinkOnTime
    if (lastBlinkOnTime==0):
        return False
    curtime = time.time()
    elapsed = curtime-lastBlinkOnTime
    if (elapsed < defTooLongOnTime):
        return False
    myprint("Turning off blink after on too long.")
    clearBlink()
    return True

def testShowBlink():
    # just put light on to start up to show its connected
    global blink
    global blinkColorStartupTest
    if (not blink):
        return
    updateLastBlinkOnTime()
    blink.fade_to_color(500, blinkColorStartupTest)
    time.sleep(0.5)
    blink.fade_to_rgb(500, 0,0,0)
    time.sleep(0.5)
    clearLastBlinkOnTime()

def blinkSetColor(colorName):
    global blink
    if (not blink):
        return
    updateLastBlinkOnTime()
    # fast clear
    blink.fade_to_rgb(250, 0,0,0)
    time.sleep(0.25)
    # fast color set
    blink.fade_to_color(250, "orange")
    time.sleep(0.25)
    # fast clear
    blink.fade_to_rgb(250, 0,0,0)
    # fade to real color
    time.sleep(0.25)
    blink.fade_to_color(500, colorName)

def clearBlink():
    global blink
    if (not blink):
        return
    clearLastBlinkOnTime()
    blink.fade_to_rgb(500, 0,0,0)
    time.sleep(0.5)

def blinkDrawAttentionProblem():
    global blinkColorNetworkIssue
    blinkSetColor(blinkColorNetworkIssue)
    clearBlink()
    blinkSetColor(blinkColorNetworkIssue)



# hotkey helpers
def setupHotkeys():
    # hotkey testing
    def resetBlink():
        myprint("Hotkey clearing blink.")
        clearBlink()
    def hotkeyExitApplication():
        global wantsQuit
        wantsQuit = True
        myprint("User requested quit using hotkey.")
    bindings = [
        [["control", "shift", "alt", "N"], None, resetBlink],
        #[["control", "shift", "alt", "C"], None, hotkeyExitApplication],
    ]
    # Register all of our keybindings
    ghotkeys.register_hotkeys(bindings) 
    # Finally, start listening for keypresses
    ghotkeys.start_checking_hotkeys()

def shutdownHotkeys():
    ghotkeys.stop_checking_hotkeys()


# output file helpers
def setupOutputFile():
    global outFile
    global outFilePath
    if (outFilePath):
        outFile = open(outFilePath, 'a', encoding="utf-8")
    if (outFile):
        myprint("Opened file for writing: " + outFilePath)
    elif (outFilePath):
        myprint("ERROR: Failed to open file for writing: " + outFilePath)

def shutdownOutputFile():
    if (outFile):
        #print("Closing chat file output gracefully.", file=outFile)        
        outFile.close()
    pass





# pytchat helpers
def setupPytchat(videoId, ytChatStreamContinuation):
    global chat, optionForceReplay
    #myprint("Connecting pytchat to: " + videoId + ".")
    if (ytChatStreamContinuation):
        #chat = pytchat.LiveChat(video_id = videoId, replay_continuation=ytChatStreamContinuation, force_replay = optionForceReplay)
        chat = pytchat.LiveChat(video_id = videoId, replay_continuation=ytChatStreamContinuation)
    else:
        #chat = pytchat.LiveChat(video_id = videoId, force_replay = optionForceReplay)
        if (optionPythonNewerPytchatKludge):
            # new python 3.10 try
            chat = pytchat.create(video_id = videoId)
        else:
            # old good
            chat = pytchat.LiveChat(video_id = videoId)

    if (chat.is_alive()):
        if (True):
            myprint("Connected to: " + videoId + ".")
        elif (chat.is_replay()):
            myprint("Connected to: " + videoId + " (replay).") 
        else:
            myprint("Connected to: " + videoId + " (LIVE).")
    else:
        myprint("Failed connecting to: " + videoId + ".")

def shutdownPytchat():
    global chat
    chat.terminate()

def loopDisplayAllChatMessages():
    global chat
    global defSleepTimeBetweenChatGets, defSleepTimeBetweenChatProcess, defSleepTimeBetweenReconnect
    global wantsQuit
    global optionReconnect
    global ytChatStreamContinuation
    global youtubeVideoId
    global outFile
    global optionUseContinuationOnReconnect, optionUseContinationEvenIfLive, optionReconnectAttemptLimit
    global optionBrief, optionFileBrief
    global reconnectAttempts
    #myprint("Fetching chat from video.")
    validChatMessages = 0
    iterationsSinceReconnect = 0
    aliveDiscoveries = 0
    reconnectAttempts = 0
    while (not wantsQuit):
        iterationsSinceReconnect = iterationsSinceReconnect + 1
        if (not chat.is_alive()):
            # try to reconnect?
            if (iterationsSinceReconnect < 3 or not optionReconnect):
                reconnectAttempts = reconnectAttempts + 1
                if (aliveDiscoveries==0) or (not optionReconnect) or (reconnectAttempts >= optionReconnectAttemptLimit):
                    myprint("Chat disconnected; stopping.")
                    if (validChatMessages==0):
                        chat.raise_for_status()
                    break
            myprint("Chat disconnected; sleeping before retry %d of %d." % (reconnectAttempts, optionReconnectAttemptLimit))
            shutdownPytchat()
            #
            blinkDrawAttentionProblem()
            loopSleepWantsQuitStop(defSleepTimeBetweenReconnect)
            clearBlink()
            #
            if (wantsQuit):
                break
            #
            setupPytchat(youtubeVideoId, ytChatStreamContinuation)
            iterationsSinceReconnect = 0
            if (not chat.is_alive()):
                continue
                # myprint("Chat still dead; aborting.")
                #break
        try:
            # reest
            aliveDiscoveries = aliveDiscoveries + 1
            # old buffered way
            if True:
                data = chat.get()
                items = data.items
            else:
                # non-buffered
                data = chat.get()
                items = data.sync_items()

            # store continuation?
            ytChatStreamContinuation = False
            if (optionUseContinuationOnReconnect):
                #if (optionUseContinationEvenIfLive or chat.is_replay()):
                #if (optionUseContinationEvenIfLive):
                if (True):
                    ytChatStreamContinuation = chat.continuation
                

            for c in items:
                # reset
                reconnectAttempts = 0
                # inc
                iterationsSinceReconnect = iterationsSinceReconnect + 1
                validChatMessages = validChatMessages + 1
                # print on screen
                if (optionBrief):
                    print(f"{c.elapsedTime} [{c.author.name}]- {c.message}")
                    #print(f"{c.datetime} [{c.author.name}]- {c.message}")
                    #print(f"{c.author.name}: {c.message}")
                else:
                    print(c.json())           
                #
                # log to file
                if (outFile):
                    if (optionFileBrief):
                        print(f"{c.elapsedTime} [{c.author.name}]- {c.message}", file=outFile)
                    else:
                        print(c.json(), file=outFile)
                #
                scanChatMessageForTriggers(c)
                #
                if (defSleepTimeBetweenChatProcess>0):
                    loopSleepWantsQuitStop(defSleepTimeBetweenChatProcess)
                if (wantsQuit):
                    break
            # done processing a batch
            if (outFile):
                outFile.flush()
            #
            turnOffBlinkIfOnTooLong()
            #
            if (wantsQuit):
                break
            loopSleepWantsQuitStop(defSleepTimeBetweenChatGets)
        except KeyboardInterrupt:
            myprint("KeyboardInterrupt hit; breaking.")
            wantsQuit = True
            break
    #myprint("Done fetching chat from video.")




def scanChatMessageForTriggers(cobj):
    # scan message for anything important
    global patternAlert, patternCancel
    
    username = cobj.author.name
    msg = cobj.message

    # cancel alert pattern
    if (patternAlert.search(msg)):
        triggerAlert()
    else:
        if (patternCancel.search(msg)):
            triggerCancelAlert()


def triggerCancelAlert():
    myprint("ALERT: Canceled.")
    clearBlink()
def triggerAlert():
    global blinkColorAlert
    myprint("ALERT: Triggered.")
    blinkSetColor(blinkColorAlert)



# main function
def main():
    myprint(appName + " v"+ appVersion)
    setup()
    # pytchat debug
    loopDisplayAllChatMessages()
    #myprint("Done. Exiting.")
    shutdown()
    exit()








# invoke main
if __name__ == "__main__":
    main()

