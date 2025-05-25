import pygame as pg
import sys

from utils.FontSys import FontSys
from player import Player
from GameWorld.GameWorld import GameWorld
from utils.Timer import Timer
from utils.Resolution import ResolutionSys
#from utils.InputManager import *
from widget.widget import Camera
class MainWindow:
    def __init__(self):
        pg.init()
        width, height = 1920, 1080
        ResolutionSys().set_res(width, height)
        self.screen = pg.display.set_mode((width, height), pg.SRCALPHA)
        pg.display.set_caption("游戏")
        self.gameWorld = GameWorld(self.screen)

        self.camera = Camera()
    def init(self):
        self.gameWorld.add_player(player=Player(x=200, y=200, screen=self.screen, surface=pg.image.load('images/kitty.jpg'),size_factor=0.1))
        self.gameWorld.set_focus(self.gameWorld.players[0])
    def exec(self):
        Timer().set_fps(120)
        running = True
        while running:
            Timer().update()

            keys = pg.key.get_pressed()
            if keys[pg.K_ESCAPE]:
                break
            for event in pg.event.get():
                if event.type == pg.QUIT:
                    running = False
                #elif event.type == pg.MOUSEMOTION:
                    #Input().set_mouse_pos(event.pos)
                #elif event.type == pg.MOUSEBUTTONDOWN:
                    #Input().set_mouse_pressed(event.button)

            # 清空屏幕
            self.screen.fill((0, 0, 0))
            # 渲染帧率
            FontSys().render_text(self.screen, "{:d}".format(int(1.0 / Timer().get_delta())))

            # 游戏物体刷新
            self.gameWorld.update(keys, Timer().get_delta(),self.camera)

            self.camera.update(self.gameWorld.get_focus_rect())
            FontSys().render_text(self.screen, f"x:{self.camera.world_rect.x},y:{self.camera.world_rect.y}",(50,0))
            self.gameWorld.render(self.camera)

            # 绘制屏幕
            pg.display.flip()


        pg.quit()
        sys.exit()


def run():
    m = MainWindow()
    m.init()
    m.exec()