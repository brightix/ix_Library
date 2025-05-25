import pygame as pg
from utils.Timer import Timer
class FontSys:
    _instance = None
    _initialized = None
    def __new__(cls):
        if not cls._instance:
            cls._instance = super().__new__(cls)
        return cls._instance

    def __init__(self):
        if self._initialized:
            return
        self._initialized = True

    def render_text(self,screen,text,pos=(0,0),color=(255,255,255,255)):
        font = pg.font.Font(None, 24)
        font_texture = font.render(text, True, color)
        screen.blit(font_texture,pos)

    def render_text_surface(self,x,y,screen,text,color=(255,255,255)):
        font = pg.font.Font(None, 24)
        font_texture = font.render(text, True, color)
        screen.blit(font_texture,(x,y))