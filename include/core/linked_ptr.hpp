/*
*This file is part of the SQUADS Library (https://github.com/eotpcomic/squads ).
*Copyright (c) 2023 Amber-Sophia Schroeck
*
*The SQUADS Library is free software; you can redistribute it and/or modify
*it under the terms of the GNU Lesser General Public License as published by
*the Free Software Foundation, version 2.1, or (at your option) any later version.

*The SQUADS Library is distributed in the hope that it will be useful, but
*WITHOUT ANY WARRANTY; without even the implied warranty of
*MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
*General Public License for more details.
*
*You should have received a copy of the GNU Lesser General Public
*License along with the SQUADS  Library; if not, see
*<https://www.gnu.org/licenses/>.
*/
#ifndef __SQUADS_LINKED_PTR_H__
#define __SQUADS_LINKED_PTR_H__

#include "config.hpp"
#include "defines.hpp"
#include "functional.hpp"
#include "algorithm.hpp"

namespace squads {

    /**
     * @brief This class allows basic_linked_ptr<T> and basic_linked_ptr<U> to
     *  share the same base nodes.
     */
    struct basic_linked_ptr_node  {
        mutable basic_linked_ptr_node* Prev;
        mutable basic_linked_ptr_node* Next;
    };

    /**
     * @brief This class implements a basic_linked_ptr template that it allows
     * sharing of pointers between instances via reference counting. basic_linked_ptr
     * objects can safely be copied
     *
     * @note Is not thread-safe.
     */
    template <typename T >
    class basic_linked_ptr : public basic_linked_ptr_node {
        template <typename U> friend class basic_linked_ptr;
    public:
        using self_type = basic_linked_ptr<T>;
        using value_type = T;
        using element_type = T;
        using reference = T&;
        using pointer = T*;
        using base_type = basic_linked_ptr_node;

        /**
         * @brief Construct a new basic linked ptr object
         */
        basic_linked_ptr()
            : m_pValue(nullptr) { Prev = Next = this; }
        /**
         * @brief Construct,
         * @note It is OK if the input pointer is null.
         */
        explicit basic_linked_ptr(pointer pValue)
            : m_pValue(pValue) { Prev = Next = this; }

        basic_linked_ptr(const basic_linked_ptr& _pPtr) : m_pValue(_pPtr.m_pValue) {
            if(m_pValue) link(_pPtr);
            else Prev = Next = this;
        }

        template <typename U>
        basic_linked_ptr(const basic_linked_ptr<U>& _pPtr)
            : m_pValue(_pPtr.m_pValue) {
                if(m_pValue) link(_pPtr);
                else Prev = Next = this;
        }
        ~basic_linked_ptr() { reset(); }


        void reset() { reset(nullptr); }
        pointer get() { return m_pValue; }

        void force_delete() { pointer _pValue = detach(); delete _pValue; }


        /**
         * @brief Returns the use count of the shared pointer.
         * @note The return value is one if the owned pointer is null.
         * @return The use count of the shared pointer.
         */
        int count() const {
            int _count = 1;

            for(const base_type* _pCurrent = static_cast<const base_type*>(this);
                    _pCurrent->Next != static_cast<const base_type*>(this);
                    _pCurrent = _pCurrent->Next) {
                ++_count;
            }

            return _count;
        }

        pointer detach() {
            T* _pValue = m_pValue;
            base_type* _p = this;

            do {
                base_type* _pNext = _p->Next;
                static_cast<self_type*>(_p)->m_pValue = 0;

                _p->Next = _p->Prev = _p;
                _p = _pNext;
            }
            while(_p != this);

            return _pValue;
        }


        /**
         * @brief If the use count of the owned pointer
         *
         * @return true The use count of the owned pointer is one
         * @return false when not
         */
        bool unique() { return (Next == static_cast<const base_type*>(this)); }

        basic_linked_ptr& operator = (const basic_linked_ptr& _pPtr) {
            if(_pPtr.m_pValue != m_pValue) {
                reset(_pPtr.m_pValue);
                if(_pPtr.m_pValue)
                    link(_pPtr);
            }
            return *this;
        }

        reference operator*() { return *m_pValue; }
        pointer operator->()  { return m_pValue; }
        operator bool ()      { return (m_pValue != 0); }

    public:
        template <typename U>
        void reset(U* pValue) {
            if(pValue != m_pValue) {
                if(unique()) {
                    delete m_pValue;
                }
                else {
                    Prev->Next  = Next;
                    Next->Prev  = Prev;
                    Prev = Next = this;
                }
                m_pValue = pValue;
            }
        }

        template <typename U>
        basic_linked_ptr& operator = (const basic_linked_ptr<U>& _pPtr) {
            if(_pPtr.m_pValue != m_pValue) {
                reset(_pPtr.m_pValue);
                if(_pPtr.m_pValue)
                    link(_pPtr);
            }
            return *this;
        }
        template <typename U>
        basic_linked_ptr& operator = (U* pValue) {
            reset(pValue);
            return *this;
        }
    protected:
        template <typename U>
        void link(const basic_linked_ptr<U>& pOther) {
            Next           = pOther.mpNext;
            Prev           = this;
            Prev           = const_cast<basic_linked_ptr<U>*>(&pOther);
            pOther.Next    = this;
        }
    protected:
        pointer m_pValue;
    };

    /**
     * @brief Make a linked pointer
     * @tparam T Value type of the pointer.
     * @tparam Args Argument for the object.
     */
    template <typename T >
    inline basic_linked_ptr<T>  make_link(const basic_linked_ptr<T>& value) {
        return basic_linked_ptr<T>(value);
    }

    /**
     * @brief Make a linked pointer from a other linked pointer type
     * @tparam T Value type of the pointer.
     * @tparam U Value of the type of td
     */
    template <typename T, typename U  >
    inline basic_linked_ptr<T>  make_link(const basic_linked_ptr<U>& value) {
        return basic_linked_ptr<T>(value);
    }

    /**
     * @brief Make a link pointer
     * @tparam T Value type of the pointer.
     * @tparam Args Argument for the object.
     */
    template <typename T>
    inline basic_linked_ptr<T>  make_link(T* value) {
        return basic_linked_ptr<T>(value);
    }


    /**
     * @brief Get the pointer object
     * @tparam T    The type of the value
     *
     * @param _pPtr  The input pointer to get the pointer
     * @return T*    The pointer of the value from input
     */
    template <typename T>
    inline T* get(const basic_linked_ptr<T>& _pPtr) {
        return _pPtr.get();
    }
    /**
     * @brief Compares two basic_linked_ptr objects for equality. Equality is defined as
     * being true when the pointer shared between two basic_linked_ptr objects is equal.
     */
    template <typename T, typename U>
    inline bool operator==(const basic_linked_ptr<T>& a, const basic_linked_ptr<U>& b) {
        return (a.get() == b.get());
    }

    /**
     * @brief Compares two basic_linked_ptr objects for inequality
     */
    template <typename T, typename U>
    inline bool operator!=(const basic_linked_ptr<T>& a, const basic_linked_ptr<U>& b) {
        return (a.get() != b.get());
    }

    template <typename T>
    using linked_ptr = basic_linked_ptr<T>;

    
}

#endif
