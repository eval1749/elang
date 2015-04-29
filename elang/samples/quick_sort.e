class Sample {
  static void QuickSort(int[] a, int m, int n) {
    if (n <= m)
      return;
    var i = m - 1;
    var j = n;
    var v = a[n];
    for (;;) {
      do { ++i; } while (a[i] < v);
      do { --j; } while (a[j] > v);
      if (i >= j) break;
      Swap(a, i, j);
    }
    Swap(a, i, n);
    QuickSort(a, m, j);
    QuickSort(a, i + 1, n);
  }

  static void Swap(int[] a, int i, int j) {
    var tmp = a[i];
    a[i] = a[j];
    a[j] = tmp;
  }
}
