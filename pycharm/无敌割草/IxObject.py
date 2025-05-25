from dataclasses import dataclass

import pygame as pg


class IxObject:
    tag_type = None
    is_instigator = False
    def __init__(self):
        self.rect = pg.Rect(0,0,0,0)
        self._surface = pg.Surface
        self._screen = None
    def render(self,camera):
        self._screen.blit(self._surface,camera.world_to_screen_pos(self.rect))
    def on_collision(self,other):
        pass

    def get_rect(self):
        return self.rect

    def get_position_center(self):
        pass