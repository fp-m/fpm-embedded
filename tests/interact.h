//
// Interact with REPL routine via input and output streams.
//
// Copyright (c) 2020 Serge Vakulenko
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
#include <sstream>
#include <future>

class Interact {
public:
    //
    // Create an interactive environment to run the given function.
    //
    explicit Interact(void (*f)(std::istream &input, std::ostream &output));
    ~Interact();

    //
    // Receive text from the function.
    //
    std::string receive();

    //
    // Send text to the function.
    //
    void send(std::string msg);

private:
    //
    // User-defined function.
    //
    void (*func)(std::istream &input, std::ostream &output);

    //
    // Future from the user task.
    //
    std::future<void> task;

    //
    // User thread starts here.
    //
    void start();

    //
    // Request to terminate the user thread.
    //
    bool terminate { false };

    //
    // Which thread is running: control or user.
    //
    enum class Mode { CONTROL, USER };
    std::atomic<Mode> mode { Mode::CONTROL };

    //
    // Switch between control thread and user thread.
    //
    void yield(Mode m);

    //
    // Condition and mutex to switch between two threads.
    //
    std::condition_variable signal;
    std::mutex mutex;

    //
    // Input buffer calls getch() method.
    //
    class Inbuf : public std::streambuf {
        Interact *session;
        char ch { '\0' };
    public:
        Inbuf(Interact *i) : session(i) {}
    protected:
        virtual int underflow() override;
    };
    Inbuf input_buf { this };

    //
    // Get next character from send_buf.
    // Activate the control thread to refill.
    // Called from Inbuf::underflow().
    //
    int getch();

    //
    // Output buffer: method send() supplies data,
    // method getch() consumes it.
    //
    std::string send_buf;
    size_t send_pos { 0 };

    //
    // Input stream from the control thread to the user task.
    //
    std::istream input_stream { &input_buf };

    //
    // Output stream from the user task to the control thread.
    //
    std::stringstream output_stream;
};
