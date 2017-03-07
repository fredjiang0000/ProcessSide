#ifndef WARNINGMESSAGE
#define WARNINGMESSAGE
using namespace std;
class warningmessage{
public:
    static bool getMessage(){
        return Result;
    }
    setWarning(bool alert){
        Result = alert;
    }
private:
    static bool Result;
};
#endif // WARNINGMESSAGE



