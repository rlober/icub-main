// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
#include <iCub/selectiveAttentionModule.h>
#include <yarp/os/Network.h>


#include <iostream>

using namespace std;
using namespace yarp::os;
using namespace yarp::sig;
using namespace yarp::sig::draw;


#define COMMAND_VOCAB_SET VOCAB3('s','e','t')
#define COMMAND_VOCAB_GET VOCAB3('g','e','t')
#define COMMAND_VOCAB_KBU VOCAB3('k','b','u') //weight of the bottom-up algorithm
#define COMMAND_VOCAB_KTD VOCAB3('k','t','d') //weight of top-down algorithm
#define COMMAND_VOCAB_RIN VOCAB3('r','i','n') //red intensity value
#define COMMAND_VOCAB_GIN VOCAB3('g','i','n') //green intensity value
#define COMMAND_VOCAB_BIN VOCAB3('b','i','n') //blue intensity value

// Image Receiver
//static YARPImgRecv *ptr_imgRecv;
// Image to Display
static yarp::sig::ImageOf<yarp::sig::PixelRgb> *ptr_inputImg=0;
// Semaphore
static yarp::os::Semaphore *ptr_semaphore;
// Timeout ID
//static guint timeout_ID;
static yarp::sig::ImageOf<yarp::sig::PixelRgb>* _outputImage;
static selectiveAttentionModule *selectiveAttentionModule;


#define _imgRecv (*(ptr_imgRecv))
#define _inputImg (*(ptr_inputImg))
#define _semaphore (*(ptr_semaphore))


bool selectiveAttentionModule::configure(ResourceFinder &rf) {
    ct = 0;
	inputImage_flag=false;
    reinit_flag=false;

	currentProcessor=0;
    inputImg=0;
    tmp=0;
    tmp2=0;

    
    
    Time::turboBoost();
    cmdPort.open(getName("/cmd:i"));
    attach(cmdPort);

    interThread=new interactionThread();
    interThread->setName(this->getName().c_str());
    printf("name:%s \n",this->getName().c_str());
    interThread->start();

    printf("\n waiting for connection of the input port \n");

    
    return true;
}

// try to interrupt any communications or resource usage
bool selectiveAttentionModule::interruptModule() {
    
    cmdPort.interrupt();
	return true;
}

bool selectiveAttentionModule::close() {
    
    printf("Closing the module ... \n");
    if(0!=currentProcessor){
        printf("Thread running! Closing the thread ... \n");
        this->currentProcessor->stop();
    }
	this->closePorts();
    printf("The module has been successfully closed ... \n");
	return true;
}

void selectiveAttentionModule::setOptions(yarp::os::Property opt){
	//options	=opt;
    // definition of the name of the module
    ConstString name=opt.find("name").asString();
    if(name!=""){
        printf("|||  Module named as :%s \n", name.c_str());
        this->setName(name.c_str());
    }
    ConstString value=opt.find("mode").asString();
    if(value!=""){
       
    }
}



void selectiveAttentionModule::createObjects() {
//    ptr_imgRecv = new YARPImgRecv;
    ptr_inputImg = new yarp::sig::ImageOf<yarp::sig::PixelRgb>;
    ptr_semaphore = new yarp::os::Semaphore;
}


bool getImage(){
	bool ret = false;
	//ret = _imgRecv.Update();
	if (ret == false){
		return false;
	}

	_semaphore.wait();
	//ret = _imgRecv.GetLastImage(&_inputImg);
	_semaphore.post();
    printf("Acquired a new image for _imgRecv /n ");
	
	//selectiveAttentionModule->processor1->inImage=&_inputImg;
	//selectiveAttentionModule->processor2->inImage=&_inputImg;
	//selectiveAttentionModule->processor3->inImage=&_inputImg;
	//printf("GetImage: out of the semaphore \n");
	//selectiveAttentionModule->inputImage_flag=true;
	return ret;
}

void selectiveAttentionModule::setUp()
{
	printf("Module setting up automatically ..../n");
}

bool selectiveAttentionModule::openPorts(){
	
    cmdPort.open(getName("cmd")); // optional command port
    attach(cmdPort); // cmdPort will work just like terminal

	return true;
}

bool selectiveAttentionModule::outPorts(){
    return true;
}

bool selectiveAttentionModule::closePorts(){
    cmdPort.close();
    printf("All the ports successfully closed ... \n");

	return true;
}



void selectiveAttentionModule::reinitialise(int width, int height){
    inputImg=new ImageOf<PixelRgb>;
    inputImg->resize(width,height);
    tmp=new ImageOf<PixelMono>;
    tmp->resize(width,height);
    tmp2=new ImageOf<PixelRgb>;
    tmp2->resize(width,height);
}


bool selectiveAttentionModule::updateModule() {
    if(0!=interThread->inputImg){
        
	    printf("input port activated! starting the processes ....\n");    

        //ConstString portName2 = options.check("name",Value("/worker2")).asString();
        //starting rgb thread and linking all the images
        startselectiveAttentionProcessor();
        interThread->map1_yarp=currentProcessor->map1_yarp;
        interThread->map2_yarp=currentProcessor->map2_yarp;
        interThread->map3_yarp=currentProcessor->map3_yarp;
        interThread->map4_yarp=currentProcessor->map4_yarp;
        interThread->map5_yarp=currentProcessor->map5_yarp;
        interThread->map6_yarp=currentProcessor->map6_yarp;
        
        interThread->linearCombinationImage=currentProcessor->linearCombinationImage;
        interThread->outputImage=currentProcessor->outputImage;

    }
    
    return true;
}

bool selectiveAttentionModule::startselectiveAttentionProcessor(){
    printf("image processor starting ..... \n");
    this->currentProcessor->start();
    return true;
}

void selectiveAttentionModule::resetFlags(){
    
}

bool selectiveAttentionModule::respond(const Bottle &command,Bottle &reply){
        
    bool ok = false;
    bool rec = false; // is the command recognized?

    mutex.wait();
    switch (command.get(0).asVocab()) {
    case COMMAND_VOCAB_HELP:
        rec = true;
        {
            reply.addString("help");

            reply.addString("\n");
            reply.addString("get fn \t: general get command \n");
            

            reply.addString("\n");
            

            reply.addString("\n");
            

            reply.addString("\n");
            reply.addString("set k1 <double> \t: setting of linear combination coefficient (map1) \n");
            reply.addString("set k2 <double> \t: setting of linear combination coefficient (map2) \n");
            reply.addString("set k3 <double> \t: setting of linear combination coefficient (map3) \n");
            reply.addString("set k4 <double> \t: setting of linear combination coefficient (map4) \n");
            reply.addString("set k5 <double> \t: setting of linear combination coefficient (map5) \n");
            reply.addString("set k6 <double> \t: setting of linear combination coefficient (map6) \n");

            reply.addString("\n");
            
            
            reply.addString("\n");


            ok = true;
        }
        break;
    case COMMAND_VOCAB_NAME:
        rec = true;
        {
            // check and change filter name to pass on to the next filter
            string fName(command.get(1).asString());
            string subName;
            Bottle subCommand;
            int pos=1;
            //int pos = fName.find_first_of(filter->getFilterName());
            if (pos == 0){
                pos = fName.find_first_of('.');
                if (pos  > -1){ // there is a subfilter name
                    subName = fName.substr(pos + 1, fName.size()-1);
                    subCommand.add(command.get(0));
                    subCommand.add(Value(subName.c_str()));
                }
                for (int i = 2; i < command.size(); i++)
                    subCommand.add(command.get(i));
                //ok = filter->respond(subCommand, reply);
            }
            else{
                printf("filter name  does not match top filter name ");
                ok = false;
            }
        }
        break;
    case COMMAND_VOCAB_SET:
        rec = true;
        {
            switch(command.get(1).asVocab()) {
            case COMMAND_VOCAB_SALIENCE_THRESHOLD:{
                double thr = command.get(2).asDouble();
            }
                break;
            case COMMAND_VOCAB_NUM_BLUR_PASSES:{
                int nb = command.get(2).asInt();
                //reply.addString("connection 2");
              
                ok=true;
            }
            break;
            case COMMAND_VOCAB_K1:{
                double w = command.get(2).asDouble();
                if(currentProcessor!=0)
                    currentProcessor->k1=w;
                ok = true;
            }
            break;
            case COMMAND_VOCAB_K2:{
                double w = command.get(2).asDouble();
                if(currentProcessor!=0)
                    currentProcessor->k2=w;
                ok = true;
            }
            break;
            case COMMAND_VOCAB_K3:{
                double w = command.get(2).asDouble();
                if(currentProcessor!=0)
                    currentProcessor->k3=w;
                ok = true;
            }
            break;
            case COMMAND_VOCAB_K4:{
                double w = command.get(2).asDouble();
                if(currentProcessor!=0)
                    currentProcessor->k4=w;
                ok = true;
            }
            break;
            case COMMAND_VOCAB_K5:{
                double w = command.get(2).asDouble();
                if(currentProcessor!=0)
                    currentProcessor->k5=w;
                ok = true;
            }
            break;
            case COMMAND_VOCAB_K6:{
                double w = command.get(2).asDouble();
                if(currentProcessor!=0)
                    currentProcessor->k6=w;
                ok = true;
            }
            break;
            case COMMAND_VOCAB_CHILD_NAME:{
                int j = command.get(2).asInt();
                string s(command.get(3).asString().c_str());
            }
                break;
            case COMMAND_VOCAB_WEIGHT:{
                double w = command.get(2).asDouble();
            }
                break;
            case COMMAND_VOCAB_CHILD_WEIGHT:{
                int j = command.get(2).asInt();
                double w = command.get(3).asDouble();
            }
                break;
            case COMMAND_VOCAB_CHILD_WEIGHTS:{
                Bottle weights;
                for (int i = 2; i < command.size(); i++)
                    weights.addDouble(command.get(i).asDouble());
            }
                break;
            default:
                cout << "received an unknown request after a SET COMMAND" << endl;
                break;
            }
        }
        break;
     
    case COMMAND_VOCAB_GET:
        rec = true;
        {
            reply.addVocab(COMMAND_VOCAB_IS);
            reply.add(command.get(1));
            switch(command.get(1).asVocab()) {
            case COMMAND_VOCAB_SALIENCE_THRESHOLD:{
                double thr=0.0;
                reply.addDouble(thr);
                ok = true;
            }
                break;
            case COMMAND_VOCAB_NUM_BLUR_PASSES:{
                int nb = 0;
                reply.addInt(nb);
                ok = true;
            }
                break;
            case COMMAND_VOCAB_NAME:{
                string s(" ");
                reply.addString(s.c_str());
                ok = true;
            }
                break;
            case COMMAND_VOCAB_CHILD_NAME:{
                int j = command.get(2).asInt();
                string s(" ");
                reply.addString(s.c_str());
                ok = true;
            }
                break;
            case COMMAND_VOCAB_CHILD_COUNT:{
                int count =0;
                reply.addInt(count);
                ok = true;
            }
                break;
            case COMMAND_VOCAB_WEIGHT:{
                double w = 0.0;
                reply.addDouble(w);
                ok = true;
            }
                break;
            case COMMAND_VOCAB_CHILD_WEIGHT:{
                int j = command.get(2).asInt();
                double w = 0.0;
                reply.addDouble(w);
                ok = true;
            }
                break;
            case COMMAND_VOCAB_CHILD_WEIGHTS:{
                Bottle weights;
                //ok = filter->getChildWeights(&weights);
                for (int k = 0; k < weights.size(); k++)
                    reply.addDouble(0.0);
            }
                break;
            default:
                cout << "received an unknown request after a GET COMMAND" << endl;
                break;
            }
        }
        break;

    }
    mutex.post();

    if (!rec)
        ok = RFModule::respond(command,reply);
    
    if (!ok) {
        reply.clear();
        reply.addVocab(COMMAND_VOCAB_FAILED);
    }
    else
        reply.addVocab(COMMAND_VOCAB_OK);

    return ok;
}
//----- end-of-file --- ( next line intentionally left blank ) ------------------
