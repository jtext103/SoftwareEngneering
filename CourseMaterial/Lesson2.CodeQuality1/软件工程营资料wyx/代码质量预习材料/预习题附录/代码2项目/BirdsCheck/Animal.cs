namespace BirdsCheck
{
    /// <summary>
    /// 定义一切动物的类
    /// </summary>
    public class Animal
    {
        public string Name;

        public bool IsOviparity;
        public bool IsHomothermal;
        public bool HasWing;
        public bool HasFeather;
        public bool CanFly;

        public Animal(){ }

        /// <summary>
        /// 给出动物属性的构造函数
        /// </summary>
        public Animal(string name, bool isOviparity, bool isHomothermal, bool hasWing, bool hasFeather, bool canFly)
        {
            Name = name;
            IsOviparity = isOviparity;
            IsHomothermal = isHomothermal;
            HasWing = hasWing;
            HasFeather = hasFeather;
            CanFly = canFly;
        }
    }
}
