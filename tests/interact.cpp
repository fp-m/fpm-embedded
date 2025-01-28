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
#include <cassert>
#include "interact.h"

//
// Create an interactive environment to run the given function.
//
Interact::Interact(void (*f)(std::istream &input, std::ostream &output))
    : func(f)
{
    std::unique_lock<std::mutex> lock(mutex);

    // Start the user task.
    mode = Mode::USER;
    task = std::async(std::launch::async, &Interact::start, this);

    // Wait until the user task switches back.
    signal.wait(lock, [=]{ return mode == Mode::CONTROL; });
    assert(mode == Mode::CONTROL);
}

//
// Stop the user task, if it's still active.
//
Interact::~Interact()
{
    if (!terminate) {
        terminate = true;
        yield(Mode::USER);
    }
}

//
// Start the thread, invoke the function.
//
void Interact::start()
{
    // Invoke the user function.
    func(input_stream, output_stream);

    // The user function has finished.
    // Set a flag for desctructor that we are done.
    terminate = true;

    // Switch to control mode.
    std::unique_lock<std::mutex> lock(mutex);
    mode = Mode::CONTROL;
    signal.notify_one();
}

//
// Switch from control thread to user thread or vise versa,
//
void Interact::yield(Mode wanted)
{
    std::unique_lock<std::mutex> lock(mutex);

    // Switch to given mode.
    assert(mode != wanted);
    mode = wanted;

    // Notify the other task.
    signal.notify_one();

    // Wait until the other task switches the mode back.
    signal.wait(lock, [=]{ return mode != wanted; });
    assert(mode != wanted);
}

//
// Send some text to the user function.
// Put it into buffer for getch().
//
void Interact::send(std::string msg)
{
    send_buf += msg;
}

//
// Receive a text from the user thread.
//
std::string Interact::receive()
{
    // Activate the user task.
    yield(Mode::USER);

    // Return the result.
    std::string result = output_stream.str();
    output_stream.str("");
    return result;
}

//
// Get a character for the user task.
// Invoked from input_stream via Inbuf::underflow().
//
int Interact::getch()
{
    for (;;) {
        if (send_pos < send_buf.size()) {
            // Get next character from send buffer.
            return send_buf[send_pos++];
        }

        // No more input data.
        // Clear the buffer.
        send_buf = "";
        send_pos = 0;

        // Switch to the control thread for more input.
        yield(Mode::CONTROL);

        // Got a request to terminate the user thread.
        if (terminate) {
            return EOF;
        }
    }
}

//
// Need more input data.
// Invoked from input_stream.
//
int Interact::Inbuf::underflow()
{
    // Get a character from send_buf.
    int result = session->getch();

    if (result != EOF) {
        // Valid character: put it into streambuf.
        ch = result;
        setg(&ch, &ch, &ch + 1);
    }
    return result;
}
