import cv2
import numpy as np
import sys
import imutils

img = cv2.imread('train3.png')
img = imutils.resize(img,900)
gray = cv2.cvtColor(img,cv2.COLOR_BGR2GRAY)
blur = cv2.GaussianBlur(gray,(7,7),0)
thresh = cv2.adaptiveThreshold(blur,255,1,1,11,2)
# dst = cv2.erode(thresh,None,iterations=1)


#################      Now finding Contours         ###################

contours,_ = cv2.findContours(thresh,cv2.RETR_EXTERNAL,cv2.CHAIN_APPROX_SIMPLE)

samples =  np.empty((0,100))
responses = []
keys = [i for i in range(48,58)]

# for cnt in contours:
#     if cv2.contourArea(cnt)>50:
#         [x,y,w,h] = cv2.boundingRect(cnt)
#         if  h>10:
#             cv2.rectangle(img,(x,y),(x+w,y+h),(0,0,255),2)

# cv2.drawContours(img, contours, -1, (0,255,0))

# cv2.imshow('img',img)
# cv2.imshow('thresh',thresh)
# cv2.waitKey(0)
# cv2.destroyAllWindows()

for cnt in contours:
    if cv2.contourArea(cnt)>50:
        [x,y,w,h] = cv2.boundingRect(cnt)
        if  h>10:
            cv2.rectangle(img,(x,y),(x+w,y+h),(0,0,255),1)
            roi = thresh[y:y+h,x:x+w]
            roismall = cv2.resize(roi,(10,10))
            cv2.imshow('norm',img)
            key = cv2.waitKey(0)

            if key == 27:  # (escape to quit)
                sys.exit()
            elif key in keys:
                responses.append(int(chr(key)))
                sample = roismall.reshape((1,100))
                samples = np.append(samples,sample,0)

responses = np.array(responses,np.float32)
responses = responses.reshape((responses.size,1))
print ("training complete")

np.savetxt('generalsamples.data',samples)
np.savetxt('generalresponses.data',responses)