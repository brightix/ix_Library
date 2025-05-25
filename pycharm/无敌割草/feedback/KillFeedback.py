from utils.EventManager import EventManager
from utils.Timer import Clock
from utils.Resolution import ResolutionSys
import pygame as pg
from pydub import AudioSegment
class KillFeedback:
    def __init__(self,screen,icon):

        pg.mixer.init()
        self.subscribe()
        self.show_clock = Clock(1.5)
        self.mul_kill_clock = Clock(3)
        self.kill_interval = Clock(0.3)
        self._screen = screen
        self._icon = icon
        w,h = ResolutionSys().get_res()
        img_w = self._icon.get_width()
        img_h = self._icon.get_height()

        self.show_pos = (w//2 - img_w//2,h*3//4 - img_h//2)
        self.kill_cnt = 0
        self.show = False
        self.mul_kill_audio = []
        self.init_audio()
    def subscribe(self):
        EventManager().subscribe("EnemyDown",self.get_killed)


    def update(self):
        #如果连杀时间没结束
        self.mul_kill_clock.update()
        self.show_clock.update()
        self.kill_interval.update()
        if self.mul_kill_clock.is_finished():
            self.kill_cnt = 0
        if self.show_clock.is_finished():
            self.show = False
    def render(self):
        if self.show:
            self._screen.blit(self._icon, self.show_pos)
    def get_killed(self,data):
        self.show_clock.reset()
        self.mul_kill_clock.reset()

        self.show = True

        if self.kill_cnt < len(self.mul_kill_audio) and self.kill_interval.is_finished():
            self.mul_kill_audio[self.kill_cnt].play()
            self.kill_interval.reset()
        else:
            print("无连杀音效")
        self.kill_cnt = min(7,self.kill_cnt+1)
        print("headShot")


    def init_audio(self):
        sound = AudioSegment.from_file("audio/mul_kill_audio.mp3")

        segment = sound[0:1100]
        segment.export("kill.wav",format="wav")
        self.mul_kill_audio.append(pg.mixer.Sound("kill.wav"))
        segment = sound[1200:2100]
        segment.export("kill.wav",format="wav")
        self.mul_kill_audio.append(pg.mixer.Sound("kill.wav"))
        segment = sound[2200:3500]
        segment.export("kill.wav",format="wav")
        self.mul_kill_audio.append(pg.mixer.Sound("kill.wav"))
        segment = sound[3600:4700]
        segment.export("kill.wav",format="wav")
        self.mul_kill_audio.append(pg.mixer.Sound("kill.wav"))
        segment = sound[4750:6400]
        segment.export("kill.wav",format="wav")
        self.mul_kill_audio.append(pg.mixer.Sound("kill.wav"))
        segment = sound[6400:8250]
        segment.export("kill.wav",format="wav")
        self.mul_kill_audio.append(pg.mixer.Sound("kill.wav"))
        segment = sound[8250:9400]
        segment.export("kill.wav",format="wav")
        self.mul_kill_audio.append(pg.mixer.Sound("kill.wav"))
        segment = sound[9450:10850]
        segment.export("kill.wav",format="wav")
        self.mul_kill_audio.append(pg.mixer.Sound("kill.wav"))
