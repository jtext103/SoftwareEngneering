namespace DllXMLTest
{
    public class LocalAdder
    {
        /// <summary>
        /// 本地计算返回两数和
        /// </summary>
        /// <param name="a"></param>
        /// <param name="b"></param>
        /// <returns></returns>
        public int AddWithXML(int a, int b)
        {
            return a + b;
        }

        //本地计算返回两数和
        public int AddWithoutXML(int a, int b)
        {
            return a + b;
        }
    }
}
