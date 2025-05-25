import pygame as pg
from IxObject import IxObject

class Quadtree:
    def __init__(self, boundary: pg.Rect, capacity=4, level=0, max_level=5):
        self.boundary = boundary
        self.capacity = capacity
        self.level = level
        self.max_level = max_level
        self.objects: list[tuple[pg.Rect, IxObject]] = []
        self.divided = False

    def clear(self):
        self.objects.clear()
        self.divided = False
        if hasattr(self, 'nw'):
            self.nw.clear()
            del self.nw
            self.ne.clear()
            del self.ne
            self.sw.clear()
            del self.sw
            self.se.clear()
            del self.se
    def update(self,new_bound):
        self.boundary = new_bound
        self.clear()
    def subdivide(self):
        x, y, w, h = self.boundary
        hw, hh = w // 2, h // 2
        self.nw = Quadtree(pg.Rect(x, y, hw, hh), self.capacity, self.level + 1, self.max_level)
        self.ne = Quadtree(pg.Rect(x + hw, y, hw, hh), self.capacity, self.level + 1, self.max_level)
        self.sw = Quadtree(pg.Rect(x, y + hh, hw, hh), self.capacity, self.level + 1, self.max_level)
        self.se = Quadtree(pg.Rect(x + hw, y + hh, hw, hh), self.capacity, self.level + 1, self.max_level)
        self.divided = True

    def insert(self, obj_rect: pg.Rect, obj: IxObject) -> bool:
        # 若不在此节点区域，则拒绝
        if not self.boundary.colliderect(obj_rect):
            return False

        # 仅叶节点存储：若未满或已到最大深度
        if len(self.objects) < self.capacity or self.level >= self.max_level:
            # 避免重复插入同一对象
            if not any(o is obj for _, o in self.objects):
                self.objects.append((obj_rect.copy(), obj))
            return True

        # 否则分裂并将对象下推
        if not self.divided:
            self.subdivide()
        inserted = False
        for quad in (self.nw, self.ne, self.sw, self.se):
            if quad.insert(obj_rect, obj):
                inserted = True
        return inserted

    def query(self, range_rect: pg.Rect, found=None) -> list:
        if found is None:
            found = []
        # 如果无交集，返回空
        if not self.boundary.colliderect(range_rect):
            return found

        # 收集当前节点
        for rect, obj in self.objects:
            if range_rect.colliderect(rect):
                if obj not in found:
                    found.append(obj)

        # 递归在子节点中查询
        if self.divided:
            self.nw.query(range_rect, found)
            self.ne.query(range_rect, found)
            self.sw.query(range_rect, found)
            self.se.query(range_rect, found)
        return found

    def check(self):
        # 本帧碰撞对去重
        handled = set()
        candidates = self.query(self.boundary)
        for obj in candidates:
            if not getattr(obj, 'is_instigator', False):
                continue
            hits = self.query(obj.get_rect())
            for other in hits:
                if other is obj:
                    continue
                pair = frozenset((obj, other))
                if pair in handled:
                    continue
                if obj.get_rect().colliderect(other.get_rect()):
                    obj.on_collision(other)
                    handled.add(pair)

    def draw(self, surface,camera):
        pg.draw.rect(surface, (0, 255, 0), camera.world_to_screen(self.boundary), 1)
        if self.divided:
            self.nw.draw(surface,camera)
            self.ne.draw(surface,camera)
            self.sw.draw(surface,camera)
            self.se.draw(surface,camera)
