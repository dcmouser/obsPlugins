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



# global app and version info
appName = "chatBlinker"
appVersion = "1.3 (2/21/23)"
appDescription = "YouTube Live Chat Blinker"
appPrintPrefix = appName + " v"+ appVersion

# global vars
wantsQuit = False

#global objects
blink = False
chat = False
ytChatStreamContinuation = False
lastBlinkOnTime = 0
outFile = False

# global configurable
youtubeVideoId = False
outFilePath = ""

# options
#defSleepTimeBetweenChatGets = 30 #30 is safe and doesnt timeout after a few hours?
defSleepTimeBetweenChatGets = 5
defSleepTimeBetweenChatProcess = 0.1
defSleepTimeBetweenReconnect = 30
defTooLongOnTime = 60
optionReconnect = True
optionMsgExtraWord = "check"

# jesse check
# cancel alert




# generic
def setup():
    global youtubeVideoId
    #
    parseCommandlineArgs()
    #
    setupOutputFile()
    setupBlink()
    setupHotkeys()
    setupPytchat(youtubeVideoId, False)
    testShowBlink()
    signal.signal(signal.SIGINT, lambda a, b: abortApplication())

def shutdown():
    shutdownPytchat()
    shutdownHotkeys()
    clearBlink()
    shutdownBlink()
    shutdownOutputFile()

def doLoopTillQuit():
    global wantsQuit
    while (not wantsQuit):
        time.sleep(0.1)

def myprint(*args, **kwargs):
    global appPrintPrefix
    print( appPrintPrefix+": " + " ".join(map(str,args)), **kwargs)

def parseCommandlineArgs():
    global appName, appDescription
    parser = argparse.ArgumentParser(prog = appName, description = appDescription)
    parser.add_argument('-v', '--videoid', required=True)
    parser.add_argument('-o', '--outfile', default = False)
    args = parser.parse_args()
    # store video id global
    global youtubeVideoId, outFilePath
    youtubeVideoId = args.videoid
    outFilePath = args.outfile


def abortApplication():
    global wantsQuit
    wantsQuit = True
    myprint("SIGINT interrupt triggering exit.")

def loopSleepWantsQuitStop(secs):
    global wantsQuit
    curtime = 0
    perSleep = 0.5
    while (curtime < secs):
        if (wantsQuit):
            break
        curtime += perSleep
        time.sleep(perSleep)
    pass



# blink helpers
def setupBlink():
    myprint("Initializing blink.")
    global blink
    blink = Blink1()
    # clear it
    clearBlink()
    #print(blink)

def shutdownBlink():
    global blink
    blink.close()
    
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
    #print(time.time())
    #print(lastBlinkOnTime)
    #print(elapsed)
    if (elapsed < defTooLongOnTime):
        return False
    myprint("Turning off blink after on too long.")
    clearBlink()
    return True

def testShowBlink():
    # just put light on to start up to show its connected
    global blink
    updateLastBlinkOnTime()
    blink.fade_to_color(500, 'green')
    time.sleep(0.5)
    blink.fade_to_rgb(500, 0,0,0)
    time.sleep(0.5)
    clearLastBlinkOnTime()

def blinkSetColor(colorName):
    global blink
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
    clearLastBlinkOnTime()
    blink.fade_to_rgb(500, 0,0,0)
    time.sleep(0.5)

def blinkDrawAttentionProblem():
    blinkSetColor("blue")
    clearBlink()
    blinkSetColor("blue")



# hotkey helpers
def setupHotkeys():
    # hotkey testing
    def resetBlink():
        myprint("Hotkey clearing blink.")
        clearBlink()
    def exitApplication():
        global wantsQuit
        wantsQuit = True
        myprint("User requested quit using hotkey.")
    bindings = [
        [["control", "shift", "alt", "N"], None, resetBlink],
        [["control", "shift", "9"], None, exitApplication],
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
        outFile = open(outFilePath, 'a')
    if (outFile):
        myprint("Opened file for writing: " + outFilePath)
    elif (outFilePath):
        myprint("ERROR: Failed to open file for writing: " + outFilePath)

def shutdownOutputFile():
    if (outFile):
        outFile.close()
    pass





# pytchat helpers
def setupPytchat(videoId, ytChatStreamContinuation):
    global chat
    myprint("Connecting pytchat to: " + videoId)
    if (ytChatStreamContinuation):
        chat = pytchat.LiveChat(video_id = videoId, replay_continuation=ytChatStreamContinuation)
    else:
        chat = pytchat.LiveChat(video_id = videoId)

def shutdownPytchat():
    global chat
    chat.terminate()

def loopDebugAllChatMessages():
    global chat
    global defSleepTimeBetweenChatGets, defSleepTimeBetweenChatProcess, defSleepTimeBetweenReconnect
    global wantsQuit
    global optionReconnect
    global ytChatStreamContinuation
    global outFile
    myprint("Fetching chat from video..")
    iterationsSinceReconnect = 0
    while (not wantsQuit):
        iterationsSinceReconnect = iterationsSinceReconnect + 1
        if (not chat.is_alive()):
            #chat.raise_for_status()
            # try to reconnect?
            if (iterationsSinceReconnect < 3 or not optionReconnect):
                myprint("Chat disconnected.. stopping.")
                break
            myprint("Chat disconnected.. sleeping before retry..")
            shutdownPytchat()
            #
            blinkDrawAttentionProblem()
            time.sleep(defSleepTimeBetweenReconnect)
            clearBlink()
            #
            global youtubeVideoId
            setupPytchat(youtubeVideoId, ytChatStreamContinuation)
            iterationsSinceReconnect = 0
            if (not chat.is_alive()):
                myprint("Chat still dead, aborting..")
                break
        try:
            data = chat.get()
            # store continuation
            ytChatStreamContinuation = chat.continuation
            #
            items = data.items
            for c in items:
                iterationsSinceReconnect = iterationsSinceReconnect + 1
                # print on screen
                print(f"{c.datetime} [{c.author.name}]- {c.message}")
                # log to file
                if (outFile):
                    print(f"{c.datetime} [{c.author.name}]- {c.message}", file=outFile)
                #
                actOnChatMessage(c.author.name, c.message)
                #
                if (defSleepTimeBetweenChatProcess>0):
                    time.sleep(defSleepTimeBetweenChatProcess)
            # done processing a batch
            if (outFile):
                outFile.flush()
            #
            turnOffBlinkIfOnTooLong()
            #
            loopSleepWantsQuitStop(defSleepTimeBetweenChatGets)
        except KeyboardInterrupt:
            myprint("KeyboardInterrupt hit; breaking.")
            wantsQuit = True
            break
    myprint("Done fetching chat from video.")




def actOnChatMessage(username, msg):
    # scan message for anything important
    global optionMsgExtraWord

    msgLower = msg.lower()

    # test, look for hello
    if (False):
        pos = msgLower.find("hello")
        if (pos!=-1):
            blinkSetColor("blue")
    # cancel blink
    pos = msgLower.find("cancel alert")
    if (pos!=-1):
        clearBlink()
    # trigger blink color on my name, but only if NOT cancel
    if (pos==-1):
        pos = msgLower.find("jesse")
        if (pos != -1):
            # also need to find the word "check"
            if (not optionMsgExtraWord) or (msgLower.find(optionMsgExtraWord)!=-1):
                blinkSetColor("red")



# main function
def main():
    myprint("Starting up.")
    setup()
    # pytchat debug
    loopDebugAllChatMessages()
    # loop
    #myprint("Looping..")
    #doLoopTillQuit()
    # end
    myprint("Exiting.")
    shutdown()








# invoke main
if __name__ == "__main__":
    main()

