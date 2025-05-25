import pygame as pg
from IxObject import IxObject
from feedback.KillFeedback import KillFeedback
from weapon.Sword import Sword
from utils.ResourceManager import ResourceManager

from utils.Timer import Timer
class Player(IxObject):
    base_angle = 0
    tag_type = "Player"
    is_instigator = True
    def __init__(self,x: float, y: float, screen, surface: pg.surface.Surface, size_factor = 1.0):
        super().__init__()
        self._x = x
        self._y = y
        self._w = surface.get_width()
        self._h = surface.get_height()

        w,h = self._w * size_factor,self._h * size_factor

        self._screen = screen
        self._show_size = (w,h)
        self._surface = pg.transform.smoothscale(surface,self._show_size)


        self.move_speed = 400
        sword = Sword(self,screen,ResourceManager().get_res_by_name(name="Sword"),size=(50,50),dis = 300)
        self.weapon = []
        for i in range(7):
            self.weapon.append(Sword(self,screen,ResourceManager().get_res_by_name(name="Sword"),size=(50,50),dis = 300))
        self.rect = pg.Rect(x,y,self._show_size[0],self._show_size[1])

        self.feedback = KillFeedback(self._screen,ResourceManager().get_res_by_name("headShot"))
    def move(self,dx: float = 0,dy: float = 0):
        self._x += dx * self.move_speed
        self._y += dy * self.move_speed
        self.rect.x = int(self._x)
        self.rect.y = int(self._y)

    def update(self):
        #draw_y = self._y
        weapon_cnt = len(self.weapon)

        angle = 360 / weapon_cnt
        for i in range(weapon_cnt):
            self.weapon[i].update((self.base_angle + i * angle) % 360,Timer().get_delta())
        self.base_angle += 3
        self.feedback.update()
    def render(self,camera):
        self._screen.blit(self._surface,camera.world_to_screen_pos(self.get_rect()))
    def render_weapon(self,camera):
        for w in self.weapon:
            w.render(camera)
    def render_feedback(self):
        self.feedback.render()
    def commit_weapon(self, quad_tree):
        for w in self.weapon:
            #print(f"[COMMIT] Weapon: {w}, rect={w.get_rect()}, is_instigator={w.is_instigator}")
            quad_tree.insert(w.get_rect(),w)
    def on_hit(self,data):
        print("玩家受到{}点伤害".format(data["damage"]))

    # 获取类
    def get_position_center(self):
        #return self._x + self._show_size[0] / 2,self._y + self._show_size[1] / 2

        return self.rect.center

    def get_position(self):    #   获取中心点
        return self._x, self._y

    def get_rect(self):
        return self.rect
    def on_collision(self,other):
        if other.tag_type == "Enemy":
            print("player get hit")