/***********************************************************
 * The DIY Flow Bench project
 * A basic flow bench to measure and display volumetric air flow using an Arduino and common automotive MAF sensor.
 * 
 * For more information please visit our GitHub project page: https://github.com/DeeEmm/DIY-Flow-Bench/wiki
 * Or join our Facebook community: https://www.facebook.com/groups/diyflowbench/
 * 
 * This project and all associated files are provided for use under the GNU GPL3 license:
 * https://github.com/DeeEmm/DIY-Flow-Bench/blob/master/LICENSE
 ***/

// Development and release version - Don't forget to update the changelog!!
using DiyFlowBench.Api.Commands;
using DiyFlowBench.Api.Responses;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace DiyFlowBench.Api.Test.Responses
{
    [TestClass]
    public class GetHumidityResponseTests
    {
        [TestMethod]
        public void GetHumidityResponse_WhenConstructed_FlowValueIsParsedCorrectly()
        {
            //Arrange
            GetHumidityCommand command = new GetHumidityCommand();
            byte[] data = ResponseHelper.AppendChecksum(new byte[] { 0x48, 0x01 });

            //Act
            GetHumidityResponse response = new GetHumidityResponse(command, data);

            //Assert
            Assert.AreEqual(0x01, response.Flow, "The humidity value is not correct.");
        }
    }
}
