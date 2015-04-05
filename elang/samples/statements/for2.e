class Sample {
  static void Main() {
    for (var a = 0; Cond(a); a = Step(a))
      Use(a);
  }
  static extern bool Cond(int a);
  static extern int Step(int a);
  static extern void Use(int a);
}
