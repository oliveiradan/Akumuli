/** 
 * PRIVATE HEADER
 *
 * Descriptions of internal data structures used to store data in memory mappaed files.
 * All data are in host byte order.
 *
 * Copyright (c) 2013 Eugene Lazin <4lazin@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

#pragma once
#include <cstdint>


namespace Akumuli {

struct EntryOffset {
    uint64_t offset;

    EntryOffset();
    EntryOffset(uint32_t offset);
    EntryOffset(const EntryOffset& other);
    EntryOffset& operator = (const EntryOffset& other);
};


/** Timestamp. Can be treated as
 *  single 64-bit value or two
 *  consequtive 32-bit values.
 */
union TimeStamp {
    uint64_t precise;
    struct t {
        uint32_t object;
        uint32_t server;
    };
};


/** Data entry. Sensor measurement, single click from
 *  clickstream and so on. Data can be variable length,
 *  timestamp can be treated as 64-bit value (high precise
 *  timestam) or as pair of two 32-bit values: object
 *  timestamp - timestamp generated by source of data (
 *  sensor or program); server timestamp - generated by
 *  Recorder by itself, this is a time of data reception.
 */
struct Entry {
    uint32_t     param_id;  //< Parameter ID
    TimeStamp        time;  //< Entry timestamp
    uint32_t       length;  //< Entry length: constant + variable sized parts
    uint32_t     value[1];  //< Data begining

    //! C-tor
    Entry(uint32_t length);

    //! Extended c-tor
    Entry(uint32_t param_id, TimeStamp time, uint32_t length);
};


//! Page types
enum PageType {
    Metadata,  //< Page with metadata used by Spatium itself
    Leaf,      //< Leaf page
    Index,     //< Index page
    Overflow   //< Overflow page
};


/**
 * In-memory page representation.
 * PageHeader represents begining of the page.
 * Entry indexes grows from low to high adresses.
 * Entries placed in the bottom of the page.
 * This class must be nonvirtual.
 */
struct PageHeader {
private:
    // metadata
    PageType type;     //< page type
    uint32_t count;    //< number of elements stored
    uint32_t length;   //< page size
    PageHeader *_prev;  //< intrusive list pointer
    PageHeader *_next;  //< intrusive list pointer
    EntryOffset 
       page_index[0];  //< page index

    //! Get const pointer to the begining of the page
    const char* cdata() const noexcept;

    //! Get pointer to the begining of the page
    char* data() noexcept;

public:
    // Intrusive list implementation
    // -----------------------------
    void insert(PageHeader* page) noexcept;
    PageHeader* next() const noexcept;
    PageHeader* prev() const noexcept;
    // -----------------------------

    //! C-tor
    PageHeader(PageType type, uint32_t count, uint32_t length);

    //! Return number of entries stored in page
    int get_entries_count() const noexcept;

    //! Returns amount of free space in bytes
    int get_free_space() const noexcept;

    //! Add operation status
    enum AddStatus {
        Success,
        Overflow,
        BadEntry
    };

    /**
     * Add new entry to page data.
     * @param entry entry
     * @returns operation status
     */
    AddStatus add_entry(Entry const& entry) noexcept;

    /**
     * Get length of the entry.
     * @param entry_index index of the entry.
     * @returns 0 if index is out of range, entry length otherwise.
     */
    int get_entry_length(int entry_index) const noexcept;

    /**
     * Copy entry from page to receiving buffer.
     * @param receiver receiving buffer
     * receiver.length must contain correct buffer length
     * buffer length can be larger than sizeof(Entry)
     * @returns 0 if index out of range, -1*entry[index].length
     * if buffer is to small, entry[index].length on success.
     */
    int copy_entry(int index, Entry* receiver) const noexcept;

    /**
     * Get pointer to entry without copying
     * @param index entry index
     * @returns pointer to entry or NULL
     */
    const Entry* find_entry(int index) const noexcept;

    /**
     * Sort page content
     */
    void sort() noexcept;
    // TODO: add partial sort
};

}  // namespaces
