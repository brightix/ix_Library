import math

from IxObject import IxObject
import pygame as pg
from utils.Timer import Clock
from utils.EventManager import EventManager
from BaseObject.Weapon import Weapon
class Sword(Weapon):
    tag_type = "Sword"
    is_instigator = True
    weapon_data = {
        "damage" : 14
    }
    attack_interval = 0.2
    def __init__(self,father: IxObject, screen,surface,size=(50,50),dis=500):
        self.father = father
        self._screen = screen
        self._surface = pg.transform.smoothscale(surface,size)
        self.size = size
        self.angle = 0.0
        self.dis = dis
        self.rect = pg.Rect(0,0,size[0],size[1])

        self.attack_clock: dict[IxObject, Clock] = {}
        self.subscribe()
        self.is_instigator = True

    def update(self,angle,delta):
        center = self.father.get_position_center()

        dx = math.sin(math.radians(angle)) * self.dis
        dy = math.cos(math.radians(angle)) * self.dis

        x = dx + center[0] - self.rect.w /2
        y = dy + center[1] - self.rect.h /2

        self.rect.x = x
        self.rect.y = y

        #更新计时器
        for _,t in self.attack_clock.items():
            t.update()
    #def follow(self,dx,dy):

    def on_collision(self, other):
        if other.tag_type == "Enemy" or other.destroyed:

            timer = self.attack_clock.get(other.uid)
            if timer is None:
                self.attack_clock[other.uid] = Clock(self.attack_interval)
                other.on_hit(self.weapon_data)
            elif timer.is_finished():
                other.on_hit(self.weapon_data)
                timer.reset()
                #print(f"sword attack{other.uid}")

    def get_rect(self):
        return self.rect

    def del_enemy(self,data):
        uid = data.get("enemy_uid")
        if uid in self.attack_clock:
            self.attack_clock.pop(uid)

    def subscribe(self):
        EventManager().subscribe("EnemyDown",self.del_enemy)