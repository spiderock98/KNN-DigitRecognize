import cv2
import numpy as np 
# import sys
import imutils
from imutils.video import VideoStream
from time import sleep
from imutils.video import FPS
import cameraindex

######################## checking MAC address #########################

# mac=open('/sys/class/net/%s/address' %'eth0').read().strip()
# while( mac!= 'b8:27:eb:df:bd:66'):
#     continue

############################ Training  ############################

samples = np.loadtxt('generalsamples4.data',np.float32)
responses = np.loadtxt('generalresponses4.data',np.float32)
responses = responses.reshape((responses.size,1))
model = cv2.ml.KNearest_create()
model.train(samples, cv2.ml.ROW_SAMPLE, responses)

############################# Init Vars #########################

refPt = [(0,0),(0,0)]
cropping = False

############################# Function  #########################
def nothing(x):
    pass

def gamma(image, gamma=1.0):
    if gamma == 0: # The minimal position of the slider is always 0.
        gamma = 1
    invGamma = 1.0 / gamma
    table = np.array([((i / 255.0) ** invGamma) * 255
        for i in np.arange(0, 256)]).astype("uint8")
    return cv2.LUT(image, table)

def click_and_crop(event, x, y, flags, param):
    global refPt, cropping
    if event == cv2.EVENT_LBUTTONDOWN:
        refPt = [(x,y),(0,0)]
        cropping = True
        
    elif event == cv2.EVENT_LBUTTONUP:
        refPt[1] = (x,y)
        cropping = False
        #print('xUp={}  yUp={}'.format(refPt[0][0],refPt[0][1]))
        #print('xDown={}  yDown={}\n'.format(refPt[1][0],refPt[1][1]))
        print('{},{},{},{}'.format(refPt[0][1],refPt[1][1],refPt[0][0],refPt[1][0]))

############################# Init I/O #########################

cv2.namedWindow("image")
cv2.namedWindow("Panel")

cv2.createTrackbar('Gamma','Panel',5,5,nothing)
cv2.createTrackbar('Gaussian','Panel',13,30,nothing)
cv2.createTrackbar('Dilate','Panel',0,10,nothing)
cv2.createTrackbar('Roi','Panel',20,100,nothing)

cv2.setMouseCallback('image', click_and_crop)

vs = VideoStream(cameraindex.index('CAMERA')).start()
vs = VideoStream(cameraindex.index('Camera')).start()

sleep(1.0)

fps = FPS().start()

imgSize = 500
############################# Loop  #########################

while True:
    im = vs.read()
    ori = im.copy()
    gammaAdj = gamma(ori,cv2.getTrackbarPos('Gamma','Panel'))
    im = gammaAdj[refPt[0][1]+1:refPt[1][1]-1,refPt[0][0]+1:refPt[1][0]-1]
    zoom = imutils.resize(im,imgSize)

    gray = cv2.cvtColor(zoom,cv2.COLOR_BGR2GRAY)
    barGauss = cv2.getTrackbarPos('Gaussian','Panel')
    gauss = cv2.GaussianBlur(gray, (barGauss*2+1,barGauss*2+1), 0)
    thresh = cv2.adaptiveThreshold(gauss,255,cv2.ADAPTIVE_THRESH_GAUSSIAN_C,cv2.THRESH_BINARY_INV,11,2)
    barDilEro = cv2.getTrackbarPos('Dilate','Panel')
    dst = cv2.dilate(thresh,None,iterations=barDilEro*2+1)
    contours,_= cv2.findContours(dst,cv2.RETR_EXTERNAL,cv2.CHAIN_APPROX_SIMPLE)

    for cnt in contours:
        if cv2.contourArea(cnt)>50:
            [x,y,w,h] = cv2.boundingRect(cnt)
            wh = cv2.getTrackbarPos('Roi','Panel')
            #if  ((w>wh-20) and (h>wh)):
            if  (h>wh):
                cv2.rectangle(zoom,(x,y),(x+w,y+h),(0,255,0),2)
                roi = dst[y:y+h,x:x+w]
                roismall = cv2.resize(roi,(10,10))
                roismall = roismall.reshape((1,100))
                roismall = np.float32(roismall)
                retval, results, neigh_resp, dists = model.findNearest(roismall, k = 5)
                val = (int((results[0][0])))
                cv2.putText(zoom,str(val),(x,y+w),0,fontScale=1,color=(0,0,255), thickness=2)

    fps.update()

    cv2.imshow('Panel',dst)
    cv2.imshow('image',ori)
    cv2.imshow('zoom',zoom)

    if cv2.waitKey(1) == 27:
        break

fps.stop()
print('FPS: {:.2f}'.format(fps.fps()))

vs.stop()
cv2.destroyAllWindows()
