#include "Analytics.h"

#include <fstream>
#include <unordered_map>

//
// Internal Functionality 
//

static Middleware::Utils::Analytics::ClientPlayoutDelayRecorder clientPlayoutputDelayRecorder;

static long long currentFramesPerSecondMeasuringSinceTime = 0;
static int totalNumberOfFramesThisSecond = 0;
static std::vector<int> numberOfFramesPerSecondAcrossApplicationLife;
static std::unordered_map<int, int> mapFromApplicationSecondToBytesSent;
static std::vector<Middleware::Utils::Analytics::PlayoutDelayBreakdown> clientPlayoutDelays;
static std::vector<Middleware::Utils::Analytics::ProcessingDelayBreakdown> serverProcessingDelays;
static std::vector<Middleware::Utils::Analytics::ServerCompressionBreakdown> serverCompressionBreakdowns;
static std::vector<Middleware::Utils::Analytics::ClientDecompressionBreakdown> clientDecompressionBreakdowns;

static void OutputClientPlayoutDelay(const std::string& outputFolderPath, int playerId) {
	// Only create an output file if data was collected.
	if(clientPlayoutDelays.size() == 0)
		return;

	std::stringstream outputFilePathStream;
	outputFilePathStream << outputFolderPath;
	outputFilePathStream << "/client_playout_delays_";
	outputFilePathStream << playerId;
	outputFilePathStream << ".csv";

	std::fstream playoutDelayFilestream(outputFilePathStream.str(), std::fstream::out);

	// Write a row explaining the data format
	playoutDelayFilestream << "\"Time To Decompress Frame (ms)\"" << ",";
	playoutDelayFilestream << "\"Time To Copy Frame (ms)\"" << ",";
	playoutDelayFilestream << "\"Time Since Frame Received (ms)\"" << ",";
	playoutDelayFilestream << "\"Time To Show Frame (ms)\"" << ",";
	playoutDelayFilestream << "\n";

	for(const Middleware::Utils::Analytics::PlayoutDelayBreakdown& delay : clientPlayoutDelays) {
		// Write the breakdown of each delay to a new row
		playoutDelayFilestream << (delay.nsToDecompressFrame / 1000000.0) << ",";
		playoutDelayFilestream << (delay.nsToCopyFrame / 1000000.0) << ",";
		playoutDelayFilestream << (delay.nsSinceFrameReceived / 1000000.0) << ",";
		playoutDelayFilestream << (delay.nsToShowFrame / 1000000.0) << ",";
		playoutDelayFilestream << "\n";
	}

	playoutDelayFilestream.close();
}

static void OutputServerProcessingDelay(const std::string& outputFolderPath) {
	// Only create an output file if data was collected.
	if(serverProcessingDelays.size() == 0) 
		return;

	std::fstream processingDelayFilestream(outputFolderPath + "/server_processing_delays.csv", std::fstream::out);

	// Write a row explaining the data format
	processingDelayFilestream << "\"Player ID\"" << ",";
	processingDelayFilestream << "\"Time Between Input Received and Rendering Starting (ms)\"" << ",";
	processingDelayFilestream << "\"Time To Render Scene For Player Group (ms)\"" << ",";
	processingDelayFilestream << "\"Time To Copy Frame From Group (ms)\"" << ",";
	processingDelayFilestream << "\"Time Between Frame Prepared and Client Handler Starting Send (ms)\"" << ",";
	processingDelayFilestream << "\"Time To Compress Frame (ms)\"" << ",";
	processingDelayFilestream << "\"Time To Send Frame (ms)\"";
	processingDelayFilestream << "\n";

	for(const Middleware::Utils::Analytics::ProcessingDelayBreakdown& delay : serverProcessingDelays) {
		// Write the breakdown of each delay to a new row
		processingDelayFilestream << delay.playerId << ",";
		processingDelayFilestream << (delay.nsSinceInputReceived / 1000000.0) << ",";
		processingDelayFilestream << (delay.nsToRenderPlayerGroupPerspectives / 1000000.0) << ",";
		processingDelayFilestream << (delay.nsToCopyPlayerPerspectiveFromGroup / 1000000.0) << ",";
		processingDelayFilestream << (delay.nsBetweenFrameCompletionAndSendStarting / 1000000.0) << ",";
		processingDelayFilestream << (delay.nsToCompressFrame / 1000000.0) << ",";
		processingDelayFilestream << (delay.nsToSendFrame / 1000000.0);
		processingDelayFilestream << "\n";
	}

	processingDelayFilestream.close();
}

static void OutputServerBytesOfFrameOfGameplaySent(const std::string& outputFolderPath) {
	// Only create an output file if data was collected.
	if(mapFromApplicationSecondToBytesSent.empty())
		return;

	std::fstream outgoingNetworkDataFilestream(outputFolderPath + "/server_bytes_of_frames_of_gameplay_sent.csv", std::fstream::out);

	// Write a row explaining the data format
	outgoingNetworkDataFilestream << "\"Seconds Since EPOCH\"" << ",";
	outgoingNetworkDataFilestream << "\"Bytes of Data Sent\"" << ",";
	outgoingNetworkDataFilestream << "\n";

	for(const auto& bytesDeliveredInSecond : mapFromApplicationSecondToBytesSent) {
		// Write the breakdown of how many bytes were delivered in a given second to a new row
		outgoingNetworkDataFilestream << bytesDeliveredInSecond.first << ",";
		outgoingNetworkDataFilestream << bytesDeliveredInSecond.second;
		outgoingNetworkDataFilestream << "\n";
	}

	outgoingNetworkDataFilestream.close();
}

static void OutputServerCompressionMeasurements(const std::string& outputFolderPath) {
	// Only create an output file if data was collected.
	if(serverCompressionBreakdowns.size() == 0)
		return;

	std::fstream compressionBreakdownFilestream(outputFolderPath + "/server_compression_breakdown.csv", std::fstream::out);

	// Write a row explaining the data format
	compressionBreakdownFilestream << "\"Frame Identifier\"" << ",";
	compressionBreakdownFilestream << "\"Decompressed Frame Size (bytes)\"" << ",";
	compressionBreakdownFilestream << "\"Compressed Frame Size (bytes)\"" << ",";
	compressionBreakdownFilestream << "\"Time To Compress (ms)\"";
	compressionBreakdownFilestream << "\n";

	for(const Middleware::Utils::Analytics::ServerCompressionBreakdown& breakdown : serverCompressionBreakdowns) {
		// Write the breakdown of the measurements to compress each frame of gameplay
		compressionBreakdownFilestream << breakdown.frameIdentifier << ",";
		compressionBreakdownFilestream << breakdown.uncompressedFrameSize << ",";
		compressionBreakdownFilestream << breakdown.compressedFrameSize << ",";
		compressionBreakdownFilestream << (breakdown.nsToCompressFrame / 1000000.0);
		compressionBreakdownFilestream << "\n";
	}

	compressionBreakdownFilestream.close();
}

static void OutputClientCompressionMeasurements(const std::string& outputFolderPath) {
	// Only create an output file if data was collected.
	if(clientDecompressionBreakdowns.size() == 0)
		return;

	std::fstream decompressionBreakdownFilestream(outputFolderPath + "/client_decompression_breakdown.csv", std::fstream::out);

	// Write a row explaining the data format
	decompressionBreakdownFilestream << "\"Frame Identifier\"" << ",";
	decompressionBreakdownFilestream << "\"Decompressed Frame Size (bytes)\"" << ",";
	decompressionBreakdownFilestream << "\"Compressed Frame Size (bytes)\"" << ",";
	decompressionBreakdownFilestream << "\"Time To Decompress (ms)\"";
	decompressionBreakdownFilestream << "\n";

	for(const Middleware::Utils::Analytics::ClientDecompressionBreakdown& breakdown : clientDecompressionBreakdowns) {
		// Write the breakdown of measurements to decompress each frame of gameplay
		decompressionBreakdownFilestream << breakdown.frameIdentifier << ",";
		decompressionBreakdownFilestream << breakdown.uncompressedFrameSize << ",";
		decompressionBreakdownFilestream << breakdown.compressedFrameSize << ",";
		decompressionBreakdownFilestream << (breakdown.nsToDecompressFrame / 1000000.0);
		decompressionBreakdownFilestream << "\n";
	}

	decompressionBreakdownFilestream.close();
}

static void OutputClientFramesPerSecondMeasurements(const std::string& outputFolderPath, int playerId) {
	// Only create an output file if data was collected.
	if(numberOfFramesPerSecondAcrossApplicationLife.size() == 0)
		return;

	std::stringstream outputFilePathStream;
	outputFilePathStream << outputFolderPath;
	outputFilePathStream << "/client_fps_";
	outputFilePathStream << playerId;
	outputFilePathStream << ".csv";

	std::fstream framesPerSecondFilestream(outputFilePathStream.str(), std::fstream::out);

	// Output a row stating the average
	float averageFramesPerSecond = 0;
	for(int currentFramesPerSecondValue : numberOfFramesPerSecondAcrossApplicationLife)
		averageFramesPerSecond += currentFramesPerSecondValue;
	averageFramesPerSecond /= numberOfFramesPerSecondAcrossApplicationLife.size();
	framesPerSecondFilestream << "\"Average Frames Per Second:\"" << "," << averageFramesPerSecond << "\n" << "\n";

	// Write a row explaining the data format
	framesPerSecondFilestream << "\"Frames Recorded Per Second\"";
	framesPerSecondFilestream << "\n";

	for(int currentFramesPerSecondValue : numberOfFramesPerSecondAcrossApplicationLife) {
		// Write the breakdown of each delay to a new row
		framesPerSecondFilestream << currentFramesPerSecondValue;
		framesPerSecondFilestream << "\n";
	}

	framesPerSecondFilestream.close();
}



//
// Analytics Functionality
//

namespace Middleware::Utils {

	Analytics::ClientPlayoutDelayRecorder& Analytics::GetClientPlayoutDelayRecorder() {
		return clientPlayoutputDelayRecorder;
	}

	void Analytics::RecordFrameOfGameplayDataSent(int bytesSent) {
		// Get the seconds since epoch
		std::chrono::steady_clock::time_point currentTimePoint = std::chrono::high_resolution_clock::now();
		long long currentSecondsSinceEpoch = std::chrono::time_point_cast<std::chrono::seconds>(currentTimePoint).time_since_epoch().count();
		
		// If the map does not contain any outgoing data measurements for the current second then add it
		auto existingValueInMap = mapFromApplicationSecondToBytesSent.find(currentSecondsSinceEpoch);
		if(existingValueInMap == mapFromApplicationSecondToBytesSent.end()) {
			mapFromApplicationSecondToBytesSent[currentSecondsSinceEpoch] = bytesSent;
		} else {
			// If the map contains outgoing data measurements for the current second then add the number of bytes sent to the existing total
			existingValueInMap->second = existingValueInMap->second + bytesSent;
		}
	}

	void Analytics::RecordServerCompressionBreakdown(int frameIdentifier, long long uncompressedSize, long long compressedSize, long long nsToCompress) {
		Analytics::ServerCompressionBreakdown breakdown = { };
		breakdown.frameIdentifier = frameIdentifier;
		breakdown.uncompressedFrameSize = uncompressedSize;
		breakdown.compressedFrameSize = compressedSize;
		breakdown.nsToCompressFrame = nsToCompress;

		serverCompressionBreakdowns.push_back(breakdown);
	}

	void Analytics::RecordClientDecompressionBreakdown(int frameIdentifier, long long uncompressedSize, long long compressedSize, long long nsToDecompress) {
		Analytics::ClientDecompressionBreakdown breakdown = { };
		breakdown.frameIdentifier = frameIdentifier;
		breakdown.uncompressedFrameSize = uncompressedSize;
		breakdown.compressedFrameSize = compressedSize;
		breakdown.nsToDecompressFrame = nsToDecompress;

		clientDecompressionBreakdowns.push_back(breakdown);
	}

	void Analytics::RecordNewFrameOfGameplayReceived() {
		// Get the seconds since epoch
		std::chrono::steady_clock::time_point currentTimePoint = std::chrono::high_resolution_clock::now();
		long long currentSecondsSinceEpoch = std::chrono::time_point_cast<std::chrono::seconds>(currentTimePoint).time_since_epoch().count();

		// If we are not already measuring how many frames have been received in a given second then start from now
		if(currentFramesPerSecondMeasuringSinceTime == 0) {
			currentFramesPerSecondMeasuringSinceTime = currentSecondsSinceEpoch;
			totalNumberOfFramesThisSecond = 0;
		} else {
			// If already measuring how many frames have been received in a given second, then if the second has changed then work out the average in the last second measured
			if(currentFramesPerSecondMeasuringSinceTime != currentSecondsSinceEpoch) {
				numberOfFramesPerSecondAcrossApplicationLife.push_back(totalNumberOfFramesThisSecond);
				totalNumberOfFramesThisSecond = 0;
				currentFramesPerSecondMeasuringSinceTime = currentSecondsSinceEpoch;
			}

			// At this point we are measuring the number of frames this second, so add this frame to the running total
			totalNumberOfFramesThisSecond++;
		}
	}

	void Analytics::OutputAnalyticsToFolder(const std::string& outputFolderPath, int playerId) {
		// Only if an output folder was provided can the analytics be output
		if(outputFolderPath.empty()) 
			return;

		OutputClientPlayoutDelay(outputFolderPath, playerId);
		OutputServerProcessingDelay(outputFolderPath);
		OutputServerBytesOfFrameOfGameplaySent(outputFolderPath);
		OutputServerCompressionMeasurements(outputFolderPath);
		OutputClientCompressionMeasurements(outputFolderPath);
		OutputClientFramesPerSecondMeasurements(outputFolderPath, playerId);
	}



	//
	// PlayerProcessingDelayRecorder class implementations
	//

	Analytics::PlayerProcessingDelayRecorder::PlayerProcessingDelayRecorder(int playerId) {
		m_DelayBreakdown.playerId = playerId;
	}

	void Analytics::PlayerProcessingDelayRecorder::RecordInputReceived() {
		// If the user sends a second input before recieving the frame for the first then technically they are still waiting for a response, so continue timing the response time
		// for their first input.
		if(m_HasInputBeenReceived)
			return;

		// Record an input being received, and start a timer to measure how long until the render process starts.
		m_HasInputBeenReceived = true;
		m_InputToRenderTimer.ResetStartTime();
	}

	void Analytics::PlayerProcessingDelayRecorder::RecordGroupRenderingStarted() {
		// Can only measure the time taken to start rendering if an input has been received and rendering has not already started.
		if(!m_HasInputBeenReceived || m_HasFrameRenderingStarted)
			return;

		// Begin timing the rendering process (and record that the rendering process has started).
		m_HasFrameRenderingStarted = true;
		m_RenderingTimer.ResetStartTime();

		// Measure time taken to start rendering a frame since an input was received.
		m_DelayBreakdown.nsSinceInputReceived = m_InputToRenderTimer.GetNanosecondsSinceStart();
	}

	void Analytics::PlayerProcessingDelayRecorder::RecordGroupRenderingComplete() {
		// Can only measure the time taken to render perspectives if an input has been received, the rendering process has started, and rendering has not already been completed.
		if(!m_HasInputBeenReceived || !m_HasFrameRenderingStarted || m_HasFrameBeenRendered)
			return;

		// Measure the time taken to render the perspectives (and record that rendering has completed).
		m_HasFrameBeenRendered = true;
		m_DelayBreakdown.nsToRenderPlayerGroupPerspectives = m_RenderingTimer.GetNanosecondsSinceStart();

		// Reset the timer to measure the time from the player perspective being rendered to being copied to the player's cache.
		m_RenderingTimer.ResetStartTime();
	}

	void Analytics::PlayerProcessingDelayRecorder::RecordPlayerFrameCopied() {
		// Can only measure the time taken to copy a rendered frame to the player's cache once an input has been received and a frame has been rendered.
		if(!m_HasInputBeenReceived || !m_HasFrameRenderingStarted || !m_HasFrameBeenRendered || m_HasFrameBeenCopied)
			return;
	
		// Measure the time taken to copy the frame to the player's cache (and record that the copy has completed).
		m_HasFrameBeenCopied = true;
		m_DelayBreakdown.nsToCopyPlayerPerspectiveFromGroup = m_RenderingTimer.GetNanosecondsSinceStart();

		// Start a timer to measure how long it takes for the rendered frame to be transmitted to the player.
		m_RenderToTransferTimer.ResetStartTime();
	}

	void Analytics::PlayerProcessingDelayRecorder::RecordTransferStarted() {
		// Can only measure the time taken to copy a rendered frame to the player's cache once an input has been received and a frame has been rendered.
		if(!m_HasInputBeenReceived || !m_HasFrameRenderingStarted || !m_HasFrameBeenRendered || !m_HasFrameBeenCopied || m_HasTransferStarted)
			return;

		// Measure the time taken for a rendered frame to start being transferred.
		m_HasTransferStarted = true;
		m_DelayBreakdown.nsBetweenFrameCompletionAndSendStarting = m_RenderToTransferTimer.GetNanosecondsSinceStart();
	}

	void Analytics::PlayerProcessingDelayRecorder::RecordCompressionStart() {
		// Can only start measuring the time taken to compress a rendered frame once an input has been received, a frame has been rendered, and a transfer has been started.
		if(!m_HasInputBeenReceived || !m_HasFrameRenderingStarted || !m_HasFrameBeenRendered || !m_HasFrameBeenCopied || !m_HasTransferStarted || m_HasCompressionStarted)
			return;
	
		// Start a timer to measure the time taken to compress the frame of gameplay
		m_HasCompressionStarted = true;
		m_TransferTimer.ResetStartTime();
	}

	void Analytics::PlayerProcessingDelayRecorder::RecordCompressionComplete() { 
		// Can only measure the time taken to compress a rendered frame once an input has been received, a frame has been rendered, and compression has been started.
		if(!m_HasInputBeenReceived || !m_HasFrameRenderingStarted || !m_HasFrameBeenRendered || !m_HasFrameBeenCopied || !m_HasTransferStarted || !m_HasCompressionStarted 
			|| m_HasCompressionCompleted)
			return;

		// Measure the time taken for a rendered frame to be compressed.
		m_HasCompressionCompleted = true;
		m_DelayBreakdown.nsToCompressFrame = m_TransferTimer.GetNanosecondsSinceStart();

		// Start a timer to measure the time taken to send the frame of gameplay.
		m_TransferTimer.ResetStartTime();
	}

	void Analytics::PlayerProcessingDelayRecorder::RecordTransferComplete() {
		// Can only measure the time taken to send a frame of gameplay when an input has been received, a frame has been rendered, and compression has finished.
		if(!m_HasInputBeenReceived || !m_HasFrameRenderingStarted || !m_HasFrameBeenRendered || !m_HasFrameBeenCopied || !m_HasTransferStarted || !m_HasCompressionStarted
			|| !m_HasCompressionCompleted)
			return;

		// Measure the time taken for a rendered frame to be compressed.
		m_DelayBreakdown.nsToSendFrame = m_TransferTimer.GetNanosecondsSinceStart();

		// Now the delays within the entire process have been recorded, so stash the measurements to be output later.
		serverProcessingDelays.push_back(m_DelayBreakdown);

		// Since the entire process has been measured, reset the recorder to measure the next process.
		m_HasInputBeenReceived = false;
		m_HasFrameRenderingStarted = false;
		m_HasFrameBeenRendered = false;
		m_HasFrameBeenCopied = false;
		m_HasTransferStarted = false;
		m_HasCompressionStarted = false;
		m_HasCompressionCompleted = false;
	}



	//
	// ClientPlayoutDelayRecorder class implementations
	//

	void Analytics::ClientPlayoutDelayRecorder::RecordFrameReceived() {
		// If a frame has already been received then continue measuring the time since that frame was received instead of resetting.
		if(m_HasFrameBeenReceived)
			return;

		// Start timing how long until the frame of gameplay is decompressed.
		m_HasFrameBeenReceived = true;
		m_ReceivedTimer.ResetStartTime();
	}

	void Analytics::ClientPlayoutDelayRecorder::RecordFrameDecompressed() {
		// To measure the time to decompress a frame, a frame must have been received and a frame cannot already have been decompressed.
		if(!m_HasFrameBeenReceived || m_HasFrameBeenDecompressed)
			return;

		// Measure the time taken to decompress the frame.
		m_HasFrameBeenDecompressed = true;
		m_DelayBreakdown.nsToDecompressFrame = m_ReceivedTimer.GetNanosecondsSinceStart();

		// Start timing how long until the frame is copied to be displayed later.
		m_ReceivedTimer.ResetStartTime();
	}

	void Analytics::ClientPlayoutDelayRecorder::RecordFrameCopied() {
		// To measure the time to copy a frame, a frame must have been received and decompressed.
		if(!m_HasFrameBeenReceived || !m_HasFrameBeenDecompressed || m_HasFrameBeenCopied)
			return;
		
		// Measure the time taken to copy the frame.
		m_HasFrameBeenCopied = true;
		m_DelayBreakdown.nsToCopyFrame = m_ReceivedTimer.GetNanosecondsSinceStart();

		// Start timing how long until the client starts rendering the frame.
		m_ReceiveToRenderTimer.ResetStartTime();
	}

	void Analytics::ClientPlayoutDelayRecorder::RecordRenderStarted() {
		// To measure the time to render a frame after being received, a frame must have been received, decompressed and copied.
		if(!m_HasFrameBeenReceived || !m_HasFrameBeenDecompressed || !m_HasFrameBeenCopied || m_HasRenderStarted)
			return;

		// Measure the time taken to begin rendering the frame received.
		m_HasRenderStarted = true;
		m_DelayBreakdown.nsSinceFrameReceived = m_ReceiveToRenderTimer.GetNanosecondsSinceStart();

		// Start timing how long until the frame is rendered.
		m_RenderTimer.ResetStartTime();
	}

	void Analytics::ClientPlayoutDelayRecorder::RecordRenderCompleted() {
		// To measure the time to render a frame, a frame must have been received, decompressed, and copied, and the rendering process must have started.
		if(!m_HasFrameBeenReceived || !m_HasFrameBeenDecompressed || !m_HasFrameBeenCopied || !m_HasRenderStarted)
			return;

		// Measure the time taken to render the frame.
		m_DelayBreakdown.nsToShowFrame = m_RenderTimer.GetNanosecondsSinceStart();

		// Now the delays within the entire process have been recorded, so stash the measurements to be output later.
		clientPlayoutDelays.push_back(m_DelayBreakdown);

		// Since the entire process has been measured, reset the recorder to measure the next process.
		m_HasFrameBeenReceived = false;
		m_HasFrameBeenDecompressed = false;
		m_HasFrameBeenCopied = false;
		m_HasRenderStarted = false;
	}

}