using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DllXMLTest
{
    public class Counter
    {
        public int SumWithXML;

        public int SumWithoutXML;

        public Counter()
        {
            DllWithXML.Adder adderWithXML = new DllWithXML.Adder();
            SumWithXML = adderWithXML.Add(1, 2);

            DllWithoutXML.Adder adderWithoutXML = new DllWithoutXML.Adder();
            SumWithoutXML = adderWithoutXML.Add(1, 2);

            LocalAdder localAdder = new LocalAdder();
            SumWithXML = localAdder.AddWithXML(1, 2);
            SumWithoutXML = localAdder.AddWithoutXML(1, 2);
        }
    }
}
