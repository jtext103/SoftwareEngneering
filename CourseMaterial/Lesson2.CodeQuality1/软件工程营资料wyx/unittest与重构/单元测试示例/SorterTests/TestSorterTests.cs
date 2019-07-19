using Microsoft.VisualStudio.TestTools.UnitTesting;
using Sorter;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Sorter.Tests
{
    [TestClass()]
    public class TestSorterTests
    {
        [TestMethod()]
        public void SortTest()
        {
            //arrange
            //准备测试数据
            int dataLength = 10;
            Random rd = new Random();

            int[] testDecreaseData = new int[dataLength];
            for(int i = 0; i < dataLength; i++)
            {
                testDecreaseData[i] = rd.Next(-1000, 1000);
            }

            int[] testIncreaseData = new int[dataLength];
            for (int i = 0; i < dataLength; i++)
            {
                testIncreaseData[i] = rd.Next(-1000, 1000);
            }

            //act
            //进行测试
            Sorter.TestSorter mySorter = new TestSorter();
            mySorter.Sort(ref testDecreaseData);
            mySorter.Sort(ref testIncreaseData, false);

            //assert
            //判断测试结果
            for(int i = 0; i < dataLength - 1; i++)
            {
                if(testDecreaseData[i] < testDecreaseData[i + 1])
                {
                    throw new Exception("Decrease Failed!");
                }
            }

            for (int i = 0; i < dataLength - 1; i++)
            {
                if (testIncreaseData[i] > testIncreaseData[i + 1])
                {
                    throw new Exception("Increase Failed!");
                }
            }
        }
    }
}