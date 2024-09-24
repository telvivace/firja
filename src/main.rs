use simple;


enum Object<'tree> {
    Free(&'tree object),
    Hit(&'tree object, Vec<object>)
}
struct object {
    x : f64,
    y : f64,
    m : u32,
    v : (f32, f32),
    s : f32,
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
    split : TreeSplit,
    left  : NodeReference<'tree>,
    right : NodeReference<'tree>,
    parent: NodeReference<'tree>,

}

fn main() {
    println!("Hello, world!");
}
