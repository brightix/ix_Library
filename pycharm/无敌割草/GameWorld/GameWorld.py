from time import CLOCK_THREAD_CPUTIME_ID

from Enemy.enemy import Enemy
from IxObject import IxObject
from player import Player

import pygame as pg
import random

from utils.EventManager import EventManager
from utils.FontSys import FontSys
from utils.Resolution import ResolutionSys
from utils.Timer import Clock
from utils.QuadTree import Quadtree
from utils.ResourceManager import ResourceManager
from Ai_level.Ai_level1 import AiLevel1

from widget.widget import Camera
class GameWorld:
    enemy_appear_clock = Clock(0)
    def __init__(self,screen):
        self.screen = screen
        self.enemies: list[Enemy] = []
        self.players: list[Player] = []
        w,h = ResolutionSys().get_res()
        self.QuadTree = Quadtree(pg.Rect(0,0,w,h))
        self.subscribe()
        self.focus :IxObject = IxObject()
    def update(self,keys,delta_sec,camera):
        #   碰撞检测
        # 在你 update 的地方
        self.QuadTree.update(camera.world_rect)
        self.enemy_appear_clock.update()
        self.refresh_enemies(camera)
        self.QuadTree.insert(self.players[0].get_rect(),self.players[0])
        for e in self.enemies:
            if camera.is_in_screen(e.get_rect()):
                self.QuadTree.insert(e.get_rect(),e)
            e.update((self.players[0].get_position()))

        #for p in self.players:
        self.player_move(keys,delta_sec)
        self.players[0].update()
        self.players[0].commit_weapon(self.QuadTree)
        self.QuadTree.check()


    def render(self,camera):

        self.players[0].render(camera)
        self.players[0].render_weapon(camera)

        for e in self.enemies:
            e.render(camera)
        self.players[0].render_feedback()
        FontSys().render_text(self.screen,f"{len(self.enemies)}",(200,0))
        self.QuadTree.draw(self.screen, camera)
    def player_move(self, keys,delta_sec):
        dx,dy = 0.0,0.0
        v = self.players[0].move_speed
        if keys[pg.K_a]:
            dx -= delta_sec
        if keys[pg.K_d]:
            dx += delta_sec
        if keys[pg.K_w]:
            dy -= delta_sec
        if keys[pg.K_s]:
            dy += delta_sec

        self.players[0].move(dx,dy)

    def add_player(self,player):    #   添加玩家
        self.players.append(player)
    def add_enemy(self,enemy):      #   添加敌人
        self.enemies.append(enemy)
    def del_enemy(self,data):

        self.enemies.remove(data["enemy"])

    def subscribe(self):
        EventManager().subscribe("EnemyDown",self.del_enemy)

    def refresh_enemies(self,camera):
        if self.enemy_appear_clock.is_finished():
            remain = 80 - len(self.enemies)
            for i in range(min(3, remain)):
                self.add_enemy(self.enemy_level_1(camera))
            self.enemy_appear_clock.set_time(random.randint(0, 3) + 1)
            self.enemy_appear_clock.reset()

    def enemy_level_1(self,camera):
        size_factor = random.uniform(0.5,1)

        x,y = self.get_offscreen_position(camera.world_rect)
        return Enemy(x,y,self.screen,ResourceManager().get_res_by_name("big_power"),AiLevel1(),size_factor,camera)

    def set_focus(self,obj: IxObject):
        self.focus = obj

    def get_offscreen_position(self, screen_rect: pg.Rect, margin=100):
        side = random.choice(["top", "bottom", "left", "right"])

        if side == "top":
            x = random.randint(screen_rect.left - margin, screen_rect.right + margin)
            y = screen_rect.top - margin
        elif side == "bottom":
            x = random.randint(screen_rect.left - margin, screen_rect.right + margin)
            y = screen_rect.bottom + margin
        elif side == "left":
            x = screen_rect.left - margin
            y = random.randint(screen_rect.top - margin, screen_rect.bottom + margin)
        elif side == "right":
            x = screen_rect.right + margin
            y = random.randint(screen_rect.top - margin, screen_rect.bottom + margin)

        return x, y

    def get_focus_rect(self):
        return self.focus.get_rect()