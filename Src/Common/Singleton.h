#ifndef __ARGENTA_SINGLETON_H__
#define __ARGENTA_SINGLETON_H__


namespace argenta
{

template<class T> class Singleton
{
public:
    static T *GetInstance()
    {   
        if (m_instance) {
            return m_instance;
        }   
        return (m_instance = new T());
    }   

private:
    static T *m_instance;

}; // class Singleton

template<class T> T *Singleton<T>::m_instance = nullptr;

} // namespace nspr

#endif // ifndef __ARGENTA_SINGLETON_H__

