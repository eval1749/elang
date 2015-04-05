// From Click's thesis Figure 6.2
using System;

class Sample {
  static void Main() {
    var i = 0;
    int a = Read(0);
    var c = 0;
    while (i < 10) {
      var b = a + 1;
      i = i + b;
      c = i * 2;
    }
    Read(c);
  }
  static extern int Read(int a);
}
