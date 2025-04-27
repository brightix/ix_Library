import cv2
import numpy as np



class InteractiveClipper:
    def __init__(self,img_path):
        self.img = cv2.imread(img_path)
        self.img = cv2.resize(self.img,None,fx=0.3,fy=0.3)
        self.startX = -1
        self.startY = -1
        self.rect_flag = False
        self.rect = (0,0,0,0)
        self.mask = np.zeros(shape=self.img.shape[:2],dtype=np.uint8)
        self.copy = self.img.copy()
        self.output = np.zeros(shape=self.img.shape[:2],dtype=np.uint8)
        self.OP_dict = \
        {
            cv2.EVENT_LBUTTONDOWN:self.handle_lb_down,
            cv2.EVENT_MOUSEMOVE:self.handle_lb_move,
            cv2.EVENT_LBUTTONUP:self.handle_lb_up
        }
    def handle_lb_down(self,x,y,flags):
        self.startX = x
        self.startY = y
        self.rect_flag = True
    def handle_lb_up(self,x,y,flags):
        self.rect_flag = False
        cv2.rectangle(self.copy, (self.startX, self.startY), (x, y), (0,0,255), 3)
        self.rect = (min(self.startX,x),min(self.startY,y),abs(self.startX - x),abs(self.startY - y))

    def handle_lb_move(self,x,y,flags):
        if self.rect_flag:
            self.copy = self.img.copy()
            cv2.rectangle(self.copy, (self.startX, self.startY), (x, y), (0,255,0), 3)



    def on_mouse(self,event,x,y,flags,params):

        if event in self.OP_dict:
            self.OP_dict[event](x,y,flags)


    def run(self):
        cv2.namedWindow('window',cv2.WINDOW_NORMAL)
        cv2.resizeWindow('window',640,480)
        cv2.setMouseCallback('window',self.on_mouse)

        while True:
            cv2.imshow('window',self.copy)
            cv2.imshow('output',self.output)

            key = cv2.waitKey(16)
            if key == ord('q'):
                break
            elif key == ord('g'):
                cv2.grabCut(self.copy,self.mask,self.rect,None,None,5,mode=cv2.GC_INIT_WITH_RECT)
            mask2 = np.where((self.mask == 1) | (self.mask == 3),255,0).astype(np.uint8)
            self.output = cv2.bitwise_and(self.copy,self.copy,mask=mask2)
        cv2.destroyAllWindows()










InteractiveClipper("./girl1.png").run()
