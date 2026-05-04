#include <thread>
#include <queue>
#include <future>
#include <unordered_map>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <functional>
#include <random>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <format>

enum TType{
    SIN,
    SQRT,
    POW
};
struct Task{
    double arg1;
    double arg2;
    TType type;
};

template <typename T>
class Server{
    std::condition_variable queue_cond_var;
    std::condition_variable_any map_cond_var;
    std::jthread server;
    std::queue<std::packaged_task<T()>> tasks;
    std::unordered_map<size_t,std::future<T>> results;
    std::mutex task_queue_mut;
    std::shared_mutex results_map_mut;
    size_t max_id=0;
    void server_thread(std::stop_token stoken)
    {
        std::packaged_task<T()> task;
        std::unique_lock<std::mutex> lock(task_queue_mut, std::defer_lock);
        while (!stoken.stop_requested())
        {
            lock.lock();
            queue_cond_var.wait(lock, [this,&stoken] { return !tasks.empty() || stoken.stop_requested(); });
            
            if (stoken.stop_requested())
            {
                break;
            }

            if (!tasks.empty())
            {
                task = std::move(tasks.front());
                tasks.pop();
                lock.unlock();
                task();
            }
        }
        output_stream << "Server stop!\n";
    }
    public:
        std::ostream& output_stream;
        Server(std::ostream& out):output_stream(out){
        }
        void start(){
            server = std::jthread([this](std::stop_token stoken) {
                server_thread(stoken);
            });
        }
        void stop(){
            server.request_stop();
            queue_cond_var.notify_one();
            server.join();
        }
        size_t add_task(struct Task task){
            std::unique_lock<std::mutex> queue_lock(task_queue_mut);
            int task_id = max_id++;
            switch (task.type)
            {
            case SIN:
                tasks.emplace(std::bind(static_cast<T(*)(T)>(std::sin),task.arg1));
                break;
            case POW:
                tasks.emplace(std::bind(static_cast<T(*)(T,T)>(std::pow),task.arg1,task.arg2));
                break;
            case SQRT:
                tasks.emplace(std::bind(static_cast<T(*)(T)>(std::sqrt),task.arg1));
            default:
                break;
            }
            std::future<T> new_future = tasks.back().get_future();
            queue_cond_var.notify_all();
            queue_lock.unlock();
            std::unique_lock<std::shared_mutex> map_lock(results_map_mut);
            results[task_id]=std::move(new_future);
            map_lock.unlock();
            map_cond_var.notify_all();
            return task_id;
        }
        T get_result(size_t id){
            std::shared_lock<std::shared_mutex> lock(results_map_mut);
            std::future<T> res_future=std::move(results[id]);
            output_stream << results.contains(id) << std::endl;
            lock.unlock();
            //output_stream << "getting result future for id " << id << std::endl;
            T res = res_future.get();
            //output_stream << "success for id " << id << std::endl;
            return res;
        }
};
void sin_thread(std::stop_token stoken,Server<double>& server,int n){
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(0.0, 100.0);
    std::ofstream out("sin_results.txt");
    for(int i=0;i<n&&!stoken.stop_requested();++i){
        struct Task task= {dist(gen),0,TType::SIN};
        size_t id = server.add_task(task);
        out << std::format("SIN({})={}",task.arg1,server.get_result(id)) << std::endl;
    }
    out.close();
    return;

}
void pow_thread(std::stop_token stoken,Server<double>& server,int n){
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(0.0, 100.0);
    std::ofstream out("pow_results.txt");
    for(int i=0;i<n&&!stoken.stop_requested();++i){
        struct Task task= {dist(gen),dist(gen),TType::POW};
        size_t id = server.add_task(task);
        out << std::format("POW({},{})={}",task.arg1,task.arg2,server.get_result(id)) << std::endl;
    }
    out.close();
    return;

}
void sqrt_thread(std::stop_token stoken,Server<double>& server,int n){
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(0.0, 100.0);
    std::ofstream out("sqrt_results.txt");
    for(int i=0;i<n&&!stoken.stop_requested();++i){
        struct Task task= {dist(gen),0,TType::SQRT};
        size_t id = server.add_task(task);
        out << std::format("SQRT({})={}",task.arg1,server.get_result(id)) << std::endl;
    }
    out.close();
    return;

}
int main(void){
    Server<double> server(std::cout);
    server.start();
    std::jthread sin_adder(sin_thread,std::ref(server),1000);
    std::jthread pow_adder(pow_thread,std::ref(server),1000);
    std::jthread sqrt_adder(sqrt_thread,std::ref(server),1000);
    sin_adder.join();
    pow_adder.join();
    sqrt_adder.join();
    server.stop();
    
    return 0;
}