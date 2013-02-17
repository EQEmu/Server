#ifndef _EQEMU_MEMORYMAPPEDFILE_H_
#define _EQEMU_MEMORYMAPPEDFILE_H_

#include <string>
#include "types.h"

namespace EQEmu {

    //! Memory Backed Shared Memory
    /*!
        Allows us to create shared memory that is backed by a file on both windows and unix platforms that 
        works in a consistent manner.  Non-copyable.
    */
    class MemoryMappedFile {
        struct Implementation;
        struct shared_memory_struct;

        //! Underlying data structure.
        struct shared_memory_struct {
            bool loaded;
            uint32 size;
            unsigned char data[1];
        };
    public:
        //! Constructor
        /*! 
            Creates a mmf for the given filename and of size.
        \param filename Actual filename of the mmf.
        \param size Size in bytes of the mmf.
        */
        MemoryMappedFile(std::string filename, uint32 size);
        
        //! Constructor
        /*! 
            Creates a mmf for the given filename and gets the size based on the existing size.
        \param filename Actual filename of the mmf.
        */
        MemoryMappedFile(std::string filename);

        //! Destructor
        ~MemoryMappedFile();

        //! Get Data Operator
        inline void *operator->() const { return memory_->data; }
        
        //! Get Data Function
        inline void *Get() const { return memory_->data; }
        
        //! Get Size Function
        inline uint32 Size() const { return memory_->size; }
        
        //! Returns whether this memory is loaded or not
        inline bool Loaded() const { return memory_->loaded; }
        
        //! Sets the memory to be loaded
        inline void SetLoaded() { memory_->loaded = true; }
        
        //! Zeros all the memory in the file, and set it to be unloaded
        void ZeroFile();
    private:
        //! Copy Constructor
        MemoryMappedFile(const MemoryMappedFile&);
        
        //! Assignment Operator
        const MemoryMappedFile& operator=(const MemoryMappedFile&);

        std::string filename_; //!< Filename of this shared memory object
        uint32 size_; //!< Size in bytes of this shared memory object
        shared_memory_struct *memory_; //!< Underlying data of the shared memory object.

        Implementation *imp_; //!< Underlying implementation.
    };
} // EQEmu

#endif
