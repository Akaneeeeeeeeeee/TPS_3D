#pragma once

#include <iostream>
#include <chrono>
#include <mutex>
#include <thread>
#include <cstdint>

class Snowflake {
private:
    static constexpr uint64_t epoch = 1609459200000ULL; // 2021-01-01 00:00:00 UTC
    static constexpr uint64_t machine_bits = 10;        // �}�V�� ID �̃r�b�g��
    static constexpr uint64_t sequence_bits = 12;       // �V�[�P���X�ԍ��̃r�b�g��
    static constexpr uint64_t max_machine_id = (1ULL << machine_bits) - 1;
    static constexpr uint64_t max_sequence = (1ULL << sequence_bits) - 1;

    uint64_t machine_id;
    uint64_t last_timestamp;
    uint64_t sequence;
    std::mutex mutex;

    // ���݂̃~���b�P�ʂ̃^�C���X�^���v���擾
    static uint64_t get_current_timestamp() {
        return static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count()
        );
    }
public:
    // �R���X�g���N�^: �}�V�� ID ���w��
    explicit Snowflake(uint64_t machine_id) : machine_id(machine_id), last_timestamp(0), sequence(0) {
        if (machine_id > max_machine_id) {
            throw std::runtime_error("Machine ID is out of range");
        }
    }

    // ��ӂ� ID �𐶐�
    uint64_t next_id() {
        std::lock_guard<std::mutex> lock(mutex);
        uint64_t timestamp = get_current_timestamp();

        if (timestamp < last_timestamp) {
            throw std::runtime_error("Clock moved backwards. Refusing to generate ID");
        }

        if (timestamp == last_timestamp) {
            sequence = (sequence + 1) & max_sequence;
            if (sequence == 0) {
                // �V�[�P���X���I�[�o�[�t���[�����玟�̃~���b��҂�
                while (timestamp <= last_timestamp) {
                    timestamp = get_current_timestamp();
                }
            }
        }
        else {
            sequence = 0;
        }

        last_timestamp = timestamp;

        // Snowflake ID ���\�z���ĕԂ� (�����Ȃ� 64 �r�b�g����)
        return ((timestamp - epoch) << (machine_bits + sequence_bits)) |
            (machine_id << sequence_bits) |
            sequence;
    }
};
