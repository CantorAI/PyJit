import os

pid = os.getpid()

import pyjit

@pyjit.object(lang="cpp",Debug=True,
              include=["CarObject.h"],
              impl=["CarObject.cpp"])
class CarObject:
    body:int ="__bind__:m_body;__default__:100"
    make:str ='__bind__:m_make;__default__:"Audi"'
    color:str ='blue'
    other:any

    #def __init__(self,x:int,y:float,z:str) -> None:
    #    pass
    def Move(self,speed:float,targetDistance:int)->int:
        pass
    def repair(self,dealer_name:str="shawn",when:str="yesterday")->bool:
        pass
    def test_func(self,x:int)->int:
        pass
    def test_func2(self,x:int,a:"CarObject")->"CarObject":
        pass

if __name__ == '__main__':
    x0 = CarObject()
    x0.make ="MyAudi"
    x0.color ="default color for CarObject"
    x = CarObject(1,10.3,'dddd')
    x.color ="this color is blue"
    cc = x.color
    x.repair('my place',"day after day")
    x.repair('second place',"Before")
    y = x.test_func(10)
    y2 = x.test_func2(10,x0)
    print("y,y2:",y,y2)
    x0 = None
    x = None
    y2 = None
    print("end")
