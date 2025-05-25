class EventManager:

    _instance = None
    _initialized = None
    def __new__(cls):
        if not cls._instance:
            cls._instance = super().__new__(cls)
        return cls._instance

    def __init__(self):
        if self._initialized:
            return
        self._listeners = {}
        self._initialized = True

    def subscribe(self,event_type,listener):
        if event_type not in self._listeners:
            self._listeners[event_type] = []
        self._listeners[event_type].append(listener)

    def unsubscribe(self,event_type,listener):
        if event_type in self._listeners:
            self._listeners[event_type].remove(listener)

    def publish(self,event_type,data=None):
        if event_type in self._listeners:
            for listener in self._listeners[event_type]:
                listener(data)
