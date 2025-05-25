from pathlib import Path
import pygame as pg
class ResourceManager:
    _instance = None
    _initialized = False
    def __new__(cls):
        if not cls._instance:
            cls._instance = super().__new__(cls)
        return cls._instance

    def __init__(self):
        if self._initialized:
            return
        self.img_lib = {}
        folder = Path("images")
        filter = [".jpg",".png"]
        for file in folder.glob("*"):
            if file.suffix in filter:
                self.img_lib[file.stem] = pg.image.load(file)
        self._initialized = True

    def get_res_by_name(self,name):
        if name in self.img_lib:
            return self.img_lib[name]
        else:
            return None
        #return self.img_lib.get(name,self.img_lib.get("default"))
        #return self.img_lib[name] if name in self.img_lib else None
