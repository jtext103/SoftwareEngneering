using System;

namespace BirdsCheck
{
    /// <summary>
    /// 定义所有鸟类的类
    /// </summary>
    public class Bird : Animal
    {
        /// <summary>
        /// 检查一个动物是否是鸟类
        /// </summary>
        static public bool CheckIfAnimalIsBird(Animal animal)
        {
            if (animal.IsOviparity == true && animal.IsHomothermal == true && animal.HasWing == true && animal.HasFeather == true)
            {
                Console.WriteLine(animal.Name + " is bird");
                return true;
            }
            else
            {
                Console.WriteLine(animal.Name + " is not bird");
                return false;
            }
        }
    }
}
