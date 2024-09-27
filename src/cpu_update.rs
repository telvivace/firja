use crate::collision_detect::Object;
use crate::collision_detect::Object::*;

fn process_hits(objects : Vec<Vec<Object>>){

    for mut buf in objects {
        for obj in buf.iter_mut(){
            match obj {
                Hit((one, other)) => {
                    //velocity of center of mass
                    let v_cm = (
                        //x velocity component
                        (one.m as f32 * one.v.0 + other.m as f32 * other.v.0)/(one.m + other.m) as f32,
                        //y velocity component
                        (one.m as f32 * one.v.1 + other.m as f32 * other.v.1)/(one.m + other.m) as f32
                    );
                    one.v = (
                        //reverse relative velocity in respect to cm
                        (one.v.0 - v_cm.0)*-1.0 + v_cm.0,
                        (one.v.1 - v_cm.1)*-1.0 + v_cm.1
                    );
                    other.v = (
                        //reverse relative velocity in respect to cm
                        (other.v.0 - v_cm.0)*-1.0 + v_cm.0,
                        (other.v.1 - v_cm.1)*-1.0 + v_cm.1
                    );
                    Free(one);
                }
                Free(_obj) => {}
            }
        }
    }
}
/*
Vec<Vec<Hit(&mut object1, &mut object2)>>

Vec<Vec<Free(&mut object1)


for enum_item in innerVec.iter_mut() {
    match enum_item {
        Hit((one, other)) => {
            ...shit
            *enum_item = Free(one);
            }
        }
    }
*/
/*
Apply their velocity to objects. Accepts delta-time.
*/
fn apply_vectors(objects: Vec<Vec<Object>>, dt:f32){
    for buf in objects{
        for obj_enum in buf {
            match obj_enum {
                Free(obj) => {
                    obj.x += (obj.v.0*dt) as f64;
                    obj.y += (obj.v.1*dt) as f64;
                }
                _ => unreachable!("apply_vectors(): not all Hit enums were converted! Look into 'process_hits()'")
            }
        }
    }
}