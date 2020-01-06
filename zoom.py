import cv2
import numpy as np
from threading import Thread
import threading


def gamma(image, gamma=1.0):
    if gamma == 0: # The minimal position of the slider is always 0.
        gamma = 1
    table = np.array([((i / 255.0) ** (1.0 / gamma)) * 255
        for i in np.arange(0, 256)]).astype("uint8")
    return cv2.LUT(image, table)


def nothing(x):
    pass


class Zoom:
    def __init__(self, ZoomSize, src, y0, y1, x0, x1):
        self.stream = cv2.VideoCapture(src)
        _, self.frame = self.stream.read()
        self.roi = self.frame[y0:y1, x0:x1]

        cv2.namedWindow('Panel{}'.format(src))
        cv2.createTrackbar('Gamma', 'Panel{}'.format(src), 0, 5, nothing)
        cv2.createTrackbar('Gaussian', 'Panel{}'.format(src), 10, 30, nothing)
        cv2.createTrackbar('Dilate', 'Panel{}'.format(src),1, 10, nothing)
        cv2.createTrackbar('Roi', 'Panel{}'.format(src),36, 100, nothing)

        self.sizeofImage = ZoomSize
        self.src = src
        self.stopped = False
        self.y0, self.y1, self.x0, self.x1 = y0, y1, x0, x1
        self.h, self.w = self.roi.shape[:2]
        self.dim = (self.sizeofImage, int(self.h * (self.sizeofImage / float(self.w))))

    def resize(self, image):
        return cv2.resize(image, self.dim, interpolation=cv2.INTER_AREA)

    def start(self):
        self.e = threading.Event()
        self.e.set() #init True
        t = Thread(target=self.update, args=(self.e,))
        t.daemon = True
        t.start()
        return self

    def update(self, e):
        while e.wait():
            # print('thread {} alive'.format(self.src))
            if self.stopped:
                return

            self.roiBar = cv2.getTrackbarPos('Roi', 'Panel{}'.format(self.src))
            _, self.frame = self.stream.read()
            if self.frame is None:
                continue
            self.roi = self.frame[self.y0:self.y1, self.x0:self.x1]
            self.zoom = self.resize(self.roi)
            self.gammaAdj = gamma(self.zoom, cv2.getTrackbarPos('Gamma', 'Panel{}'.format(self.src)))
            self.barGauss = cv2.getTrackbarPos('Gaussian', 'Panel{}'.format(self.src))
            self.gauss = cv2.GaussianBlur(self.gammaAdj, (self.barGauss * 2 + 1, self.barGauss * 2 + 1), 0)
            self.gray = cv2.cvtColor(self.gauss, cv2.COLOR_BGR2GRAY)
            self.thresh = cv2.adaptiveThreshold(self.gray, 255, cv2.ADAPTIVE_THRESH_GAUSSIAN_C, cv2.THRESH_BINARY_INV, 11, 2)
            self.dst = cv2.dilate(self.thresh, None, iterations=cv2.getTrackbarPos('Dilate', 'Panel{}'.format(self.src)))

    def readDST(self):
        return self.dst

    def readFrame(self):
        return self.frame

    def readZoom(self):
        return self.zoom

    def pause(self):
        self.e.clear()

    def cont(self):
        self.e.set()

    def stop(self):
        self.stopped = True
