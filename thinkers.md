- update f
    - collision detect:
        * bruteforce
        * BSP tree
    - seclevel update (spreminja vektorje) IZPRAZNI VEKTORJE HITOV!!!!
    - firstlevel update (spreminja skalarje)
- render f
    - read thru tree:
        * for every thing blip it on screen

- struct: 
    {
        x:  f64,
        y:  f64,
        m:  u32,
        v:  (x:f32,y:f32),
        s:  f32,
    }
