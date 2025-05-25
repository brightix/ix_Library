import pygame as pg

from utils.Resolution import ResolutionSys


class Widget:
    def __init__(self,w,h,screen):
        self.rect = pg.Rect(0,0,w,h)
        self.camera = pg.Surface((w,h))
        self.screen = screen
        self.items = {}
    def update(self):
        self.screen.blit(self.camera,self.rect.topleft)

    # def add_item(self):
    #     for item in self.items:
class Camera:
    def __init__(self):
        w,h = ResolutionSys().get_res()
        self.screen_rect = pg.Rect(0,0,w,h)
        self.world_rect = pg.Rect(0,0,w,h)
    def is_in_screen(self,query_rect):
        return query_rect.colliderect(self.world_rect)
    def update(self,owner_rect):
        self.world_rect.x = owner_rect.x + owner_rect.w // 2 - self.screen_rect.w // 2
        self.world_rect.y = owner_rect.y + owner_rect.h // 2- self.screen_rect.h // 2
    def world_to_screen_pos(self,world_rect):
        return world_rect.x - self.world_rect.x, world_rect.y - self.world_rect.y
    def world_to_screen(self,world_rect):
        return pg.Rect(world_rect.x - self.world_rect.x, world_rect.y - self.world_rect.y,world_rect.w,world_rect.h)