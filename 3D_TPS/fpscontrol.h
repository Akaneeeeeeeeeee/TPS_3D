#pragma once
#include <chrono>
#include <thread>
#include <iostream>
#include <cstdint>

/**
 * @class FPS
 * @brief �w�肵��FPS���[�g�ŏ����𐧌䂷�邽�߂̃��[�e�B���e�B�N���X
 *
 * �w�肳�ꂽFPS�Ɋ�Â��ăt���[�����Ƃ̑ҋ@�������s�����ƂŁA
 * �`��Ȃǂ̏��������肵���t���[�����[�g�Ŏ��s�ł���悤�ɂ��܂��B
 */
class FPS {
public:
    /**
     * @brief �f�t�H���g�R���X�g���N�^�͋֎~
     */
    FPS() = delete;

    /**
     * @brief FPS���w�肵�ď�����
     * @param fps �ڕW�Ƃ���t���[�����b��
     */
    explicit FPS(uint64_t fps)
        : m_MicrosecondsPerFrame(1000000 / fps),
        m_last_time(std::chrono::steady_clock::now())
    {
    }

    /**
     * @brief �O���Tick()����̌o�ߎ��Ԃ��}�C�N���b�P�ʂŎ擾�im_last_time�͍X�V���Ȃ��j
     * @return �o�ߎ��ԁi�}�C�N���b�j
     */
    uint64_t CalcDelta() {
        auto now = std::chrono::steady_clock::now();
        auto delta_us = std::chrono::duration_cast<std::chrono::microseconds> (now - m_last_time).count();
        return delta_us;
    }

    /**
     * @brief �c�莞�Ԃ�����ꍇ�̂݃X���[�v����FPS���ێ�
     *
     * @note m_delta_time �� Tick() �ōX�V����Ă��邱�Ƃ��O��B
     * �t���[���������Ԃ��w��FPS���������ꍇ�isleep_us <= 0�j�̓X���[�v���܂���B
     */
    void Wait() const {
        int64_t sleep_us = static_cast<int64_t>(m_MicrosecondsPerFrame) - static_cast<int64_t>(m_delta_time);
        if (sleep_us > 0) {
            std::this_thread::sleep_for(std::chrono::microseconds(sleep_us));
        }
        // sleep_us <= 0 �Ȃ�t���[�����x��Ă��邽�߁A�X���[�v�͍s��Ȃ�
    }

    /**
     * @brief Tick���Ăяo�����ƂŁA���̃t���[���܂ł̑ҋ@��delta_time�̍X�V���s��
     *
     * ���̊֐��ɂ��A�O�t���[������̌o�ߎ��Ԃ��v�����A�ڕWFPS���Z���ꍇ�ɂ͑ҋ@���܂��B
     * �Čv����� m_last_time ���X�V���܂��B
     */
    void Tick() {
        auto now = std::chrono::steady_clock::now();
        auto delta_us = std::chrono::duration_cast<std::chrono::microseconds>(now - m_last_time).count();
        int64_t sleep_us = static_cast<int64_t>(m_MicrosecondsPerFrame) - static_cast<int64_t>(delta_us);
        if (sleep_us > 0) {
            std::this_thread::sleep_for(std::chrono::microseconds(sleep_us));
            now = std::chrono::steady_clock::now(); // �X���[�v��ɍĎ擾
            delta_us = std::chrono::duration_cast<std::chrono::microseconds>(now - m_last_time).count();
        }
        m_last_time = now;
        m_delta_time = delta_us;
    }

    /*
        /// @brief �y�������zCalcDelta: �Ăяo������m_last_time���X�V����delta_time��Ԃ�
        /// @return �o�ߎ��ԁi�}�C�N���b�j
        ///
        /// @bug ���̎����ł� Wait() �Ăяo���O�� m_last_time ���X�V���Ă��܂����߁A
        /// ���ۂ̑ҋ@���Ԃ��Z���v�Z����A�w��FPS�ȏ�ɑ����Ȃ�o�O������܂����B
        /// Tick() �̂悤�ɁA�X���[�v��ɐ��m�Ȏ������擾���čX�V����K�v������܂��B

        uint64_t CalcDelta() {
            auto now = std::chrono::steady_clock::now();
            auto delta_us = std::chrono::duration_cast<std::chrono::microseconds>(now - m_last_time).count();
            m_last_time = now;
            m_delta_time = delta_us;
            return m_delta_time;
        }
    */

    /*
        /// @brief �y�������zWait: �O���CalcDelta�œ���m_delta_time�Ɋ�Â��đҋ@
        ///
        /// @bug CalcDelta() �� m_last_time ���X�V���Ă��܂����߁A
        /// Wait() �ł͌Â������g���Ă��܂����m�ȑҋ@���Ԃ������܂���ł����B
        /// �܂��ATick() ���������ꂽ���Ƃŏ����̐��������ۂ����悤�ɉ��P����܂����B

        void Wait() const {
            int64_t sleep_us = static_cast<int64_t>(m_MicrosecondsPerFrame) - static_cast<int64_t>(m_delta_time);
            if (sleep_us > 0) {
    #if defined(DEBUG) || defined(_DEBUG)
                std::cout << "Sleep time: " << sleep_us / 1000.0f << " ms" << std::endl;
    #endif
            }
            std::this_thread::sleep_for(std::chrono::microseconds(sleep_us));
        }
    */

private:
    uint64_t m_MicrosecondsPerFrame = 0;  ///< 1�t���[��������̖ڕW���ԁi�}�C�N���b�j
    uint64_t m_delta_time = 0;            ///< �O�t���[������̌o�ߎ��ԁi�}�C�N���b�j
    std::chrono::steady_clock::time_point m_last_time; ///< �O��Tick�܂��͏���������
};
