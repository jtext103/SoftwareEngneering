using System;

namespace BirdsCheck
{
    class Program
    {
        static void Main(string[] args)
        {
            Animal penguin = new Animal("penguin", true, true, true, true, false);
            Animal eagle = new Animal("eagle", true, true, true, true, true);
            Animal chicken = new Animal("chicken", true, true, true, true, false);

            Bird.CheckIfAnimalIsBird(penguin);
            Bird.CheckIfAnimalIsBird(eagle);
            Bird.CheckIfAnimalIsBird(chicken);

            Console.ReadKey(false);     //黑框显示出来后，输入一个字符才会关闭，便于查看输出结果
        }
    }
}
