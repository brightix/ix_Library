import random

import pygame as pg
from Ai_level.Ai_level import AiLevel
from IxObject import IxObject
from utils.EventManager import EventManager
from utils.FontSys import FontSys
from utils.Timer import Clock


class Enemy(IxObject):
    tag_type = "Enemy"
    data = {
        "damage" : 10
    }
    _id_counter = 0
    def __init__(self,x: float,y: float,screen,surface: pg.surface.Surface,ai: AiLevel,size_factor,camera):
        super().__init__()
        self.blackboard = {
            "x": x,
            "y": y,
            "hp":100.0,
            "move_speed" : random.uniform(1,2),
            "t_x":None,
            "t_y":None
        }
        self.ai = ai
        self._screen = screen

        self._w = surface.get_width()
        self._h = surface.get_height()
        w, h = self._w * size_factor,self._h * size_factor
        self._surface = pg.transform.smoothscale(surface,(w,h))
        self.subscribe()
        self.rect = pg.Rect(x,y,w,h)
        self.touch_player_clock = Clock(0)
        self.uid = Enemy._id_counter
        Enemy._id_counter += 1
        self.destroyed = False
        self.camera = camera
        self.get_hit = False
    #每次更新敌人都需要获取玩家的位置
    def update(self,target: tuple):
        self.blackboard["t_x"],self.blackboard["t_y"] = target[0],target[1]
        self.ai.update(blackboard=self.blackboard)
        self.rect.x = int(self.blackboard["x"])
        self.rect.y = int(self.blackboard["y"])
        self.touch_player_clock.update()

    def render(self,camera):
        if not camera.is_in_screen(self.get_rect()):
            return
        x,y = camera.world_to_screen_pos(self.get_rect())
        self._screen.blit(self._surface,(x,y))
        if self.get_hit:
            self.get_hit = False
            pg.draw.rect(self._screen, (255, 0, 0), self.camera.world_to_screen(self.get_rect()), 4)

        FontSys().render_text_surface(x,y,self._screen,str(self.blackboard["hp"]))
        #print(self.rect)
        #print(self.blackboard["hp"])

    def get_position_center(self):
        pass
        #return self.blackboard["x"], self.blackboard["y"]

    def get_position(self):
        return self.blackboard["x"], self.blackboard["y"]

    def get_rect(self):
        return self.rect

    def subscribe(self):
        EventManager().subscribe("enemy_get_hit", self.on_hit)

    #   受击
    def on_hit(self,data):
        #print("受到 {:d} 点伤害,还有{}点血".format(data.get("damage",-1),self.blackboard["hp"]))
        self.blackboard["hp"] -= data["damage"]

        if self.blackboard["hp"] <= 0:
            self.destroy()
        else:
            self.get_hit = True


    def on_collision(self,other):
        b = self.touch_player_clock.is_finished()
        if b and other.tag_type == "Player":
            other.on_hit(self.data)
            self.touch_player_clock = Clock(1.0)

    def destroy(self):
        if not self.destroyed:
            self.destroyed = True
            data = {"enemy" : self,"enemy_uid" : self.uid}
            EventManager().publish("EnemyDown",data)