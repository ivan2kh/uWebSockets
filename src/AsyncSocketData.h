/*
 * Authored by Alex Hultman, 2018-2019.
 * Intellectual property of third-party.

 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at

 *     http://www.apache.org/licenses/LICENSE-2.0

 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef UWS_ASYNCSOCKETDATA_H
#define UWS_ASYNCSOCKETDATA_H

#include <string>
#include <list>

namespace uWS {

/* Depending on how we want AsyncSocket to function, this will need to change */

template <bool SSL>
struct AsyncSocketData {
    struct Deque {
        typedef char T;
        typedef std::vector<T> Chunk;
    public:
        Deque() {};
        size_t length() {return len;};
        void clear() {all.clear(); len=0;}
        void append(const T *buf, size_t len) {
            appendInternal(buf, len);
            this->len += len;
        }

        void reserve(size_t) {};
        void popBeginning(size_t count) {
            len = count>len ? 0 : len - count;
            chunkSize = count>32? count : 32;
            auto it = all.begin();
            for(;it!=all.end();it++) {
                if(it->size() <= count) {
                    count -= it->size();
                    continue;
                } else {
                    it->erase(it->begin(), it->begin()+count);
                    break;
                }
            }
            all.erase(all.begin(), it);
        }

        std::pair<T*, size_t> getBeginning() {
            if(all.empty()) {
                return { nullptr, 0};
            } else {
                auto x = all.begin();
                return {x->data(), x->size()};
            }
        }

    private:
        void appendInternal(const T *buf, size_t len) {
            if(!len) {
                return;
            }
            auto pushBack = [&](){
                size_t feed = std::min(len, chunkSize);
                all.emplace_back(Chunk(buf, buf+feed));
                appendInternal(buf+feed, len-feed);
            };
            if(all.empty()) {
                pushBack();
            } else {
                Chunk &chunk = all.back();
                size_t has = chunk.size();
                if(has<chunkSize) {
                    size_t feed = std::min(len, chunkSize-has);
                    chunk.insert(chunk.end(), buf, buf+feed);
                    appendInternal(buf+feed, len-feed);
                } else {
                    pushBack();
                }
            }
        }

//  struct Chunk {
//    Chunk(size_t) {}
//    Chunk(const Chunk &) = delete;
//    Chunk(Chunk &&) = delete;
//    Chunk& operator=(const Chunk&) = delete;
//    Chunk& operator=(Chunk&&) = delete;
//    T *buf;
//    size_t len;
//  };

        std::list<Chunk> all;
        size_t len=0;
        size_t chunkSize=16*1024;
    };

    /* This will do for now */
    Deque buffer;

    /* Allow move constructing us */
    AsyncSocketData(Deque &&backpressure) : buffer(std::move(backpressure)) {

    }

    /* Or emppty */
    AsyncSocketData() = default;
};

}

#endif // UWS_ASYNCSOCKETDATA_H
