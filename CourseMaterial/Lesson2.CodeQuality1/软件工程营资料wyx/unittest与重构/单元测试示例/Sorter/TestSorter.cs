using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Sorter
{
    public class TestSorter
    {
        /// <summary>
        /// 冒泡排序
        /// </summary>
        /// <param name="numbers">需要排序的数组，排序完毕后会重置</param>
        /// <param name="decrease">默认降序，升序给 false</param>
        public void Sort(ref int[] numbers, bool decrease = true)
        {
            int middle;
            if(numbers.Length <= 1)
            {
                return;
            }
            for(int i = 0; i < numbers.Length - 1; i++)
            {
                for(int j = 0; j < numbers.Length - i - 1; j++)
                {
                    if(decrease == true)
                    {
                        if (numbers[j] < numbers[j + 1])
                        {
                            middle = numbers[j];
                            numbers[j] = numbers[j + 1];
                            numbers[j + 1] = middle;
                        }
                    }
                    else
                    {
                        if (numbers[j] > numbers[j + 1])
                        {
                            middle = numbers[j];
                            numbers[j] = numbers[j + 1];
                            numbers[j + 1] = middle;
                        }
                    }
                }
            }
        }
    }
}
