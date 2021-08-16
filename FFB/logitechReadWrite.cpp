#include "logitechReadWrite.h"

#define LOOPTIME_MS (LONGLONG)10
#define DURATION_MS (LONGLONG)5000
__int8 steeringAngle, throttlePos, brakePos;
ULONGLONG refTime;
ULONGLONG const loopTime = 8;
int fixedLatency, latencySD, latencyMean;
std::normal_distribution<float> randGenerator( 200, 50 );
std::default_random_engine uniformRand;
LARGE_INTEGER frequency;
int participantNumber;




class messageQueue
{
public:

	driverMessage* first, *last;
	driverMessage messagePool[QUEUELENGTH];
	ULONGLONG msgCounter = 0;
	messageQueue() {
		for (int i = 0; i < QUEUELENGTH-1; i++)
		{
			messagePool[i].next = &messagePool[i + 1];
		}
		messagePool[QUEUELENGTH - 1].next = &messagePool[0];
		//driverMessage initialMessage = { 0, 0, 0, {}, &messagePool[1] , TRUE };
		//messagePool[0] = initialMessage;
		first = &messagePool[0];
		last = &messagePool[0];
	};
	void enqueue(driverMessage newMessage) {
		if (last->next == first) {
			printf("queue is full, message dropped\n");
			return;
		}

		if (!newMessage.latestRead)
		{
			this->last->latestRead = FALSE;
			printf("duplicate message received, message dropped\n");
			return;
		}
		newMessage.next = (last->next)->next;
		newMessage.messageID = ++msgCounter;
		*(last->next) = newMessage;
		last = last->next;
	};
	driverMessage* returnFirst() {
		return first;
	}
	driverMessage* dequeue() {
		if (first == last) {
			printf("only one message left in the queue, dequeue failed\n");
			return first;
		}
		driverMessage* firstMessage = first;
		first = first->next;
		return firstMessage;
	}
	driverMessage* returnWithLatency(LONGLONG& latency) {
		LARGE_INTEGER HRCurrentTime;
		LONGLONG currentTime;
		getHRTimeMS(&HRCurrentTime, &frequency, currentTime);
		while (currentTime - first->time > latency)
			dequeue();
		return first;
	}
};
void getHRTimeMS(LARGE_INTEGER* HRPtr, LARGE_INTEGER* frequency, LONGLONG& timeMS) {
	QueryPerformanceCounter(HRPtr);
	timeMS = (HRPtr->QuadPart * 1000) / (frequency->QuadPart);
}
int (*latencySelect)();
int constLatency() {
	return fixedLatency;
}
int variableLatency() {
	return (int)randGenerator(uniformRand);
}





int main() {
	
	initWinsock();
	bindSocket();
	char fileNameBuf[200];
	std::string fileName;
	std::ofstream logFile;
	//logFile.open("D:\\LogitechStandalone\\LogitechStandalone\\FFB\\log.txt" , std::fstream::out);
	std::cout << "enter particiapnt number :";
	std::cin >> participantNumber;
	std::cout << "variable or fixed latency (v/f):  ";
	char testType; 
	BOOL inputCheck = FALSE;
	

	while (!inputCheck) {
		std::cin >> testType;
		switch (testType)
		{
		case 'v':
			inputCheck = TRUE;
			std::cout << "\nvariable latency selected, insert mean latency: ";
			std::cin >> latencyMean;
			std::cout << "\n insert standard deviation: ";
			std::cin >> latencySD;
			printf("\nlatency set to normal distribution with mean %d and standard deviation %d\n", latencyMean, latencySD);
			randGenerator = std::normal_distribution<float>(latencyMean, latencySD);
			latencySelect = variableLatency;
			system("pause");
			snprintf(fileNameBuf, sizeof(fileNameBuf),
				"D:\\LogitechStandalone\\LogitechStandalone\\FFB\\logFiles\\p%d_variablem%dsd%d.txt", participantNumber, latencyMean, latencySD);
			fileName = fileNameBuf;
			break;
		case 'f':
			inputCheck = TRUE;
			std::cout << "\nfixed latency selected, insert latency in ms: ";
			std::cin >> fixedLatency;
			std::cout << "\nlatency set to " << fixedLatency << " ms\n";
			latencySelect = constLatency;
			system("pause");
			//snprintf(fileNameBuf, sizeof(fileNameBuf),
			//	"D:\\LogitechStandalone\\LogitechStandalone\\FFB\\logfiles\\p%d_constant%d.txt", participantNumber, fixedLatency);
			snprintf(fileNameBuf, sizeof(fileNameBuf), "logfiles\\p%d_constant%d.txt", participantNumber, fixedLatency);
			fileName = fileNameBuf;
			break;

		default:
			std::cout << "please enter f or v\n";
			break;
		}
	}

	std::cout << "file name is " << fileName << std::endl;
	logFile.open(fileName, std::fstream::out);
	messageQueue msgQ = messageQueue();
	
	//ULONGLONG counter;
	
	LONGLONG loopStartTime, processStartTime, endTime, elapsedTime;
	LARGE_INTEGER HRProcessStartTime, HRloopStartTime, HRendTime, HRelapsedTime;
	QueryPerformanceFrequency(&frequency);
	getHRTimeMS(&HRloopStartTime, &frequency, loopStartTime);
	getHRTimeMS(&HRProcessStartTime, &frequency, processStartTime);
	getHRTimeMS(&HRendTime, &frequency, endTime);
	getHRTimeMS(&HRelapsedTime, &frequency, elapsedTime);
	driverMessage msg1 = { 20, 50, processStartTime, {}, 0,0, TRUE };
	msgQ.enqueue(msg1);
	LONGLONG latencyMS = latencySelect();
	LONGLONG lastLatencyUpdateTime = processStartTime;
	logFile << "time  message_ID     message_latency     reference_latency\n";
	while(true)
	{
		// all the data logging goes here 
		getHRTimeMS(&HRloopStartTime, &frequency, loopStartTime);
		//msgQ.enqueue(getDriverInputs(processStartTime, frequency));
		driverMessage msg = { 20, 50, loopStartTime, {}, 0,0, TRUE };
		msgQ.enqueue(msg);
		driverMessage* currentMsg = msgQ.returnWithLatency(latencyMS);
		logFile << elapsedTime - processStartTime << "          "<<
			currentMsg->messageID<<"        "<<
			elapsedTime  - currentMsg->time<<"           "<<
			 latencyMS<< std::endl;
		

		//update latency 
		if (loopStartTime - lastLatencyUpdateTime > latencyMS) {
			latencyMS = latencySelect();
			lastLatencyUpdateTime = loopStartTime;
		}




		while (endTime - loopStartTime < LOOPTIME_MS){
			getHRTimeMS(&HRendTime, &frequency, endTime);
		}
		getHRTimeMS(&HRelapsedTime, &frequency, elapsedTime);
	}



}
