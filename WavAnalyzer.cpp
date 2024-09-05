#include "WavAnalyzer.h"
#include <iostream>

void WavAnalyzer::Open()
{
	if (!File.is_open())
	{
        File.open(PathToWav, std::ios::binary | std::ios::in);
		if (!File.is_open())
		{
			std::cout << "\nWrong file path or name!";
		}
	}
}

void WavAnalyzer::Close()
{
	if (File.is_open())
	{
		File.close();
	}
}

// �������� ������ ��� �������� ��������� 4 ����
// TODO: ����������� ��������� ��������� ����� ��������
float WavAnalyzer::GetAmplitude(bool* BuffSample)
{
	// ������ ��� ��������� ����������� 4 ����
	float Amplitude = *((float*)BuffSample);
	return Amplitude;
}

WavAnalyzer::~WavAnalyzer()
{
	if (File.is_open())
	{
		Close();
	}
}

WavAnalyzer::AnalyzeRes WavAnalyzer::Analyze()
{
	AnalyzeRes Res{};
	std::vector<AnalayzeData> ChannelsData;
	Open();
	if (!File.is_open())
	{
		std::cout << "\nFile is not open!";
		return Res;
	}
	File.read((char*)&Header, sizeof(WavHeader));
	for (int i = 0; i < Header.numChannels; ++i)
	{
		Res.ResArr.push_back({ 0,0 });
		ChannelsData.push_back({});
	}
	blocksAmm = Header.subchunk2Size / Header.blockAlign;
	bool* BuffBlock = new bool[Header.bitsPerSample * Header.numChannels];
    for (unsigned int i = 0; i < blocksAmm; ++i)
	{
		File.read((char*)BuffBlock, Header.blockAlign);
		for (int j = 0; j < Header.numChannels; ++j)
		{
			// �������� ������ ��� ��������� �������� 4 ����
			float Amplitude = GetAmplitude(BuffBlock + j * Header.bitsPerSample / 8);
			// ������� ������� ������� ��������� ��������, ����� ��� ��� ������ ��� ���� ����� 0, ������ ������ ��������� �������� 3 �������, ��������� 2
			++ChannelsData[j].CounterSamples;
			// ���������, ����� ��������� �������� ����� 0
            if ((Amplitude == 0.0f) || (Amplitude < -0.0f) && (ChannelsData[j].LastAmplitude > +0.0f)
                || (Amplitude > +0.0f) && (ChannelsData[j].LastAmplitude < -0.0f))
			{
				if (++ChannelsData[j].CounterViaZero == 3)
				{
					ChannelsData[j].CounterViaZero = 1;
					if (ChannelsData[j].CounterSamples > ChannelsData[j].MaxSamples or ChannelsData[j].MaxSamples == 0)
					{
						ChannelsData[j].MaxSamples = ChannelsData[j].CounterSamples;
					}
					if (ChannelsData[j].CounterSamples < ChannelsData[j].MinSamples or ChannelsData[j].MinSamples == 0)
					{
						ChannelsData[j].MinSamples = ChannelsData[j].CounterSamples;
					}
					ChannelsData[j].CounterSamples = 1;
				}
			}
			ChannelsData[j].LastAmplitude = Amplitude;
		}
	}
	Close();
	for (int i = 0; i < Header.numChannels; ++i)
	{
		// �.�. ������� 44100 ������� � �������, �� ������� ����� ������� ���������, 
		// �������� ��� � ���� ������� �� 44100.
		// �.�. ������� = 1/T, ��� T = N/t, � ��� t = (��� ��� ���� �������)/44100, � ��� N = 1,
		// �� ������� ��������� = 44100/(��� ��� ���� �������)
		Res.ResArr[i].first = (float)Header.sampleRate / (float)ChannelsData[i].MinSamples;
		Res.ResArr[i].second = (float)Header.sampleRate / (float)ChannelsData[i].MaxSamples;
	}
	delete[] BuffBlock;
	return Res;
}
