int main() {
  // return 12312;
  // return +(--!6); // 看起来像个颜文字
  // return (1 + 2 / 2 + 3 * 4) % 7;
  // return 0 && 0 && 1 || 1 && -!0;
  // return (20 != 90) || (-!((1 * 8) && (2 + 0)));
  // return ((1&& 1) || (22 && 1)) && (10 || 0);
  // const int x = 1 + 2 + 3 + 4, y = 1 != 2,
  //           z = (20 != 90) || (-!((1 * 8) && (2 + 0)));
  // int bac = 10;
  // a = (a + 1 * 30 + 2 / 10 + 3) * 2;
  // bac = bac + 1;
  // n = a - b;
  // return bac;
  // return !((-1)+(+2)-(!3)*0) + 1000 * 8000;
  // return 2 && 4;
  int a = 10;
  if (a > 1)
    if (a > 2)
      if (a < 3)
        return a;
      else if (a > 4)
        if (a < 5)
          return a + 1;
        else
          return a + 2;
  return -1;
  // 返回值是7
}