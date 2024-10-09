use crate::collision_detect::Object;
use crate::collision_detect::Object::*;
use crate::collision_detect::object;
/*
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
                    *obj = Free(one);
                }
                Free(one) => {}
            }
        }
    }
}
    */
fn process_hits2(objects:&mut Vec<Vec<Object>>) -> () {
    for buf in objects {
        let mut index = 0;
        let mut obj_enum;
        loop {
            if index < buf.len(){
                index += 1;
            } else {
                break;
            }
            obj_enum = &mut buf[index];
            if let Hit((one, other)) = *obj_enum {
                let v_cm = (
                    //x velocity component
                    (one.m as f32 * one.v.0 + other.m as f32 * other.v.0)/(one.m + other.m) as f32,
                    //y velocity component
                    (one.m as f32 * one.v.1 + other.m as f32 * other.v.1)/(one.m + other.m) as f32
                );
                let one_updated = object {
                    v : (
                        //reverse relative velocity in respect to cm
                        (one.v.0 - v_cm.0)*-1.0 + v_cm.0,
                        (one.v.1 - v_cm.1)*-1.0 + v_cm.1
                        ),
                        ..*one
                };
                let other_updated = object {
                    v : (
                        //reverse relative velocity in respect to cm
                        (other.v.0 - v_cm.0)*-1.0 + v_cm.0,
                        (other.v.1 - v_cm.1)*-1.0 + v_cm.1
                    ),
                    ..*other
                };
                *one = one_updated;
                *other = other_updated;
                *obj_enum =  Free(one);
            }
            
            /*
            let new_enum = Free( crate::object {
                
            });
            */
        }
    }
}
fn process_hits3<'tree>(buf: &mut Vec<Object>) -> (){
        for obj in buf {
            let newobj = match *obj {
                Free(one) => one,
                Hit((one, _)) => one,
            };
            *obj = Free(newobj);
        }
}
fn process_hits4(buf : &mut Vec<Object>) -> () {
    for obj in buf.iter_mut() {
    }
}
/*
Apply velocity to objects. Accepts delta-time.
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