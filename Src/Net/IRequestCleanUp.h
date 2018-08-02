#ifndef __ARGENTA_I_REQUEST_CLEAN_UP_H_INCLUDED__
#define __ARGENTA_I_REQUEST_CLEAN_UP_H_INCLUDED__


namespace argenta
{

class IRequestCleanUp
{
public:
	IRequestCleanUp() {};
	virtual ~IRequestCleanUp() {};

    virtual void OnCleanUp() = 0;

}; // class IRequestCleanUp

} // namespace argenta

#endif // ifndef __ARGENTA_I_REQUEST_CLEAN_UP_H_INCLUDED__

