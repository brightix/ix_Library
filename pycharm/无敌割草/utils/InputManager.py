class Input:
    _instance = None
    _initialized = False
    def __new__(cls):
        if not cls._instance:
            cls._instance = super().__new__(cls)
        return cls._instance

    def __init__(self):
        if self._initialized:
            return

        self.mouse_pos = (0,0)
        self._initialized = True
        self.keys = {}
    def update(self,events):
        self.keys[]
    def set_mouse_pos(self,pos):
        self.mouse_pos = pos

    def get_mouse_pos(self):
        return self.mouse_pos

    def set_mouse_pressed(self, button):
        self.keys[button] = True