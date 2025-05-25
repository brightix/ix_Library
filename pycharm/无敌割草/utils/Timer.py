import pygame as pg
class Timer:
    _instance = None
    _initialized = False
    def __new__(cls):
        if not cls._instance:
            cls._instance = super().__new__(cls)
        return cls._instance

    def __init__(self):

        if self._initialized:
            return
        self.tick = float(0.0)
        self.fps = 60
        self.clock = pg.time.Clock()
        self.delta = 1
        self._initialized = True
    #def init(self,fps: int):
    #    self.fps = fps

    def update(self):
        self.delta = self.clock.tick(self.fps) / 1000.0
        self.tick += self.delta
    #   获取类
    def get_delta(self):
        return self.delta
    def get_tick(self):
        return self.tick
    #   设置类
    def set_fps(self, fps: int):
        self.fps = fps

class Clock:
    def __init__(self,t):
        self.elapsed = t
        self._t = t
    def update(self):
        self.elapsed += Timer().get_delta()

    def is_finished(self):
        return self.elapsed >= self._t

    def reset(self):
        self.elapsed = 0.0

    def set_time(self,t):
        self._t = t