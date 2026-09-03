provider atlas {
    probe task_enqueue(uint64_t new_size);
    probe task_dequeue(uint64_t new_size);

    probe task_execution_start(
        const char* file,
        unsigned int line,
        const char* function
    );
    probe task_execution_end(
        const char* file,
        unsigned int line,
        const char* function
    );
};
