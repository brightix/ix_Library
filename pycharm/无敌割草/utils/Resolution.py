class ResolutionSys:
    _instance = None
    _initialized = False

    width,height = 0,0
    def __new__(cls):
        if not cls._instance:
            cls._instance = super().__new__(cls)
        return cls._instance

    def __init__(self):
        if self._initialized:
            return

        self._initialized = True

    def set_res(self,w,h):
        self.width = w
        self.height = h

    def get_res(self):
        return self.width,self.height
