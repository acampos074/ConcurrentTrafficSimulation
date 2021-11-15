#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */




template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    std::unique_lock<std::mutex> lck(_mtx);
    // use a lambda expresion for the wait function
    _condVar.wait(lck,[this] { return !_queue.empty(); }); // wait until queue is not empty
    T _message = std::move(_queue.back());
    _queue.clear();
    
    return _message;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> lck(_mtx); 
    _queue.emplace_back(std::move(msg)); // use move semantics since msg is an rvalue
    _condVar.notify_one();

}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while(true){
        TrafficLightPhase _currentPhase = _msgQueue.receive();
        if(_currentPhase == TrafficLightPhase::green){
            return;
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread 
    // when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 

    // pick a random cycle duration between 4 and 6 seconds
    std::default_random_engine generator;
    std::uniform_real_distribution<double> distribution(4.0, 6.0);
    // initalize variables
    double cycleDuration = 1000.0*distribution(generator); // duration of a cycle duration in ms
    std::chrono::time_point<std::chrono::system_clock> lastUpdate;

    // init stop watch
    lastUpdate = std::chrono::system_clock::now();
    while(true){
        // sleep for 1ms at every iteration to reduce CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        // compute time difference to stop watch
        long timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - lastUpdate).count();
        if (timeSinceLastUpdate >= cycleDuration){
            // toggles the current phase of the traffic light between red and green
            _currentPhase = getCurrentPhase() == TrafficLightPhase::red ? TrafficLightPhase::green : TrafficLightPhase::red; 
            // sends an update method to the message queue using move semantics
            _msgQueue.send(std::move(getCurrentPhase()));
            // reset stop watch for next cycle
            lastUpdate = std::chrono::system_clock::now();
            // sample random value
            cycleDuration = 1000.0*distribution(generator); // duration of a cycle duration in ms
        }
    } // eof simulation loop
}

