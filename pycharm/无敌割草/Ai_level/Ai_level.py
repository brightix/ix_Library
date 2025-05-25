from abc import abstractmethod

class AiLevel:
    @abstractmethod
    def update(self,blackboard):
        pass