const PLANE: (f64, f64) = (100f64, 100f64);
pub enum Object<'tree> {
    Free(&'tree mut object),
    Hit((&'tree mut object, &'tree mut object))
}

pub struct object {
    pub x: f64,
    pub y: f64,
    pub m: u32,
    pub v: (f32, f32),
    pub s: f32,
}
enum NodeReference<'tree> {
    Node(&'tree TreeNode<'tree>),
    Buf(&'tree Vec<object>),
    Stub
}
enum TreeSplit {
    X(f64),
    Y(f64),
}
enum TreeNode<'tree> {
    Left(treeNode<'tree>),
    Right(treeNode<'tree>)
}
struct treeNode<'tree> {
    split: TreeSplit,
    left: NodeReference<'tree>,
    right: NodeReference<'tree>,
    parent: NodeReference<'tree>,
}

impl<'tree> treeNode<'tree> {
    fn new() -> Self {
        treeNode {
            split: TreeSplit::X(PLANE.0 / 2f64),
            left: NodeReference::Stub,
            right: NodeReference::Stub,
            parent: NodeReference::Stub,
        }
    }

    //The root is unique in that it has Stub as a parent
    fn is_root(&self) -> bool {
        matches!(self.parent, NodeReference::Stub)
    }
}
