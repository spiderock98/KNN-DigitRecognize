#import subprocess
import os


def index(cameraID):
    #strDevice = str(subprocess.check_output("v4l2-ctl --list-devices", shell=True))
    strDevice = os.popen('v4l2-ctl --list-devices').read()
    camID = strDevice[strDevice.find(cameraID):]
    camID = camID[camID.find('video'):]
    return int(camID[5])