#ifndef MIDDLEWARE__ANALYTICS__H
	#define MIDDLEWARE__PERFORMANCE__H
	
	#include "Timings.h"

	namespace Middleware::Utils::Analytics {

		struct ProcessingDelayBreakdown {
			int playerId = 0;

			long long nsSinceInputReceived = 0;

			long long nsToRenderPlayerGroupPerspectives = 0;
			long long nsToCopyPlayerPerspectiveFromGroup = 0;

			long long nsBetweenFrameCompletionAndSendStarting = 0;
			long long nsToCompressFrame = 0;
			long long nsToSendFrame = 0;
		};

		class PlayerProcessingDelayRecorder {
			public:
				PlayerProcessingDelayRecorder(int playerId);

				void RecordInputReceived();

				void RecordGroupRenderingStarted();
				void RecordGroupRenderingComplete();
				void RecordPlayerFrameCopied();

				void RecordTransferStarted();
				void RecordCompressionStart();
				void RecordCompressionComplete();
				void RecordTransferComplete();

			private:
				bool m_HasInputBeenReceived = false;
				Timer m_InputToRenderTimer;

				bool m_HasFrameRenderingStarted = false;
				bool m_HasFrameBeenRendered = false;
				bool m_HasFrameBeenCopied = false;
				Timer m_RenderingTimer;
				Timer m_RenderToTransferTimer;

				bool m_HasTransferStarted = false;
				bool m_HasCompressionStarted = false;
				bool m_HasCompressionCompleted = false;
				Timer m_TransferTimer;

				ProcessingDelayBreakdown m_DelayBreakdown;
		};

		struct PlayoutDelayBreakdown {
			long long nsToDecompressFrame = 0;
			long long nsToCopyFrame = 0;

			long long nsSinceFrameReceived = 0;
			long long nsToShowFrame = 0;
		};

		class ClientPlayoutDelayRecorder {
			public:
				void RecordFrameReceived();
				void RecordFrameDecompressed();
				void RecordFrameCopied();

				void RecordRenderStarted();
				void RecordRenderCompleted();

			private:
				bool m_HasFrameBeenReceived = false;
				bool m_HasFrameBeenDecompressed = false;
				bool m_HasFrameBeenCopied = false;
				Timer m_ReceivedTimer;
				Timer m_ReceiveToRenderTimer;

				bool m_HasRenderStarted = false;
				Timer m_RenderTimer;

				PlayoutDelayBreakdown m_DelayBreakdown;
		};

		ClientPlayoutDelayRecorder& GetClientPlayoutDelayRecorder();

		struct ServerCompressionBreakdown {
			int frameIdentifier = 0;
			long long uncompressedFrameSize = 0;
			long long compressedFrameSize = 0;
			long long nsToCompressFrame = 0;
		};

		struct ClientDecompressionBreakdown {
			int frameIdentifier = 0;
			long long uncompressedFrameSize = 0;
			long long compressedFrameSize = 0;
			long long nsToDecompressFrame = 0;
		};

		void RecordFrameOfGameplayDataSent(int bytesSent);
		void RecordServerCompressionBreakdown(int frameIdentifier, long long uncompressedSize, long long compressedSize, long long nsToCompress);
		void RecordClientDecompressionBreakdown(int frameIdentifier, long long uncompressedSize, long long compressedSize, long long nsToDecompress);
		void RecordNewFrameOfGameplayReceived();

		void OutputAnalyticsToFolder(const std::string& outputFolderPath, int playerId = 0);

	}

#endif