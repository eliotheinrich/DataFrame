#pragma once

#include "utils.hpp"

#include <vector>
#include <string>
#include <math.h>
#include <numeric>

namespace dataframe {

class Sample {
	public:
		Sample() : mean(0.), std(0.), num_samples(0) {}
        Sample(double mean) : mean(mean), std(0.), num_samples(1) {}
		Sample(double mean, double std, uint32_t num_samples) : mean(mean), std(std), num_samples(num_samples) {}

		template<class T>
		Sample(const std::vector<T> &v) {
			num_samples = v.size();
			mean = std::accumulate(v.begin(), v.end(), 0.0);
			double sum = 0.0;
			for (auto const t : v) {
				sum += std::pow(t - mean, 2.0);
			}

			std = std::sqrt(sum/(num_samples - 1.));
		}

		Sample(const std::string &s) {
			if (s.front() == '[' && s.back() == ']') {
				std::string trimmed = s.substr(1, s.length() - 2);
				std::vector<uint32_t> pos;
				for (uint32_t i = 0; i < trimmed.length(); i++) {
					if (trimmed[i] == ',')
						pos.push_back(i);
				}

				if (pos.size() != 2)
					throw std::invalid_argument("Invalid string provided to Sample(std::string).");

				mean = std::stof(trimmed.substr(0, pos[0]));
				std = std::stof(trimmed.substr(pos[0]+1, pos[1]));
				num_samples = std::stoi(trimmed.substr(pos[1]+1, trimmed.length()-1));
			} else {
				mean = std::stof(s);
				std = 0.;
				num_samples = 1;
			}
		}

        double get_mean() const {
			return this->mean;
		}
		void set_mean(double mean) {
			this->mean = mean;
		}

        double get_std() const {
			return this->std;
		}
		void set_std(double std) {
			this->std = std;
		}

        uint32_t get_num_samples() const {
			return this->num_samples;
		}
		void set_num_samples(uint32_t num_samples) {
			this->num_samples = num_samples;
		}

        Sample combine(const Sample &other) const {
			uint32_t combined_samples = this->num_samples + other.get_num_samples();
			if (combined_samples == 0) return Sample();
			
			double samples1f = get_num_samples(); double samples2f = other.get_num_samples();
			double combined_samplesf = combined_samples;

			double combined_mean = (samples1f*this->get_mean() + samples2f*other.get_mean())/combined_samplesf;
			double combined_std = std::pow((samples1f*(std::pow(this->get_std(), 2) + std::pow(this->get_mean() - combined_mean, 2))
											+ samples2f*(std::pow(other.get_std(), 2) + std::pow(other.get_mean() - combined_mean, 2))
											)/combined_samplesf, 0.5);

			return Sample(combined_mean, combined_std, combined_samples);
		}

		//static std::vector<double> get_means(const std::vector<Sample> &samples) {
		//	std::vector<double> v;
		//	for (auto const &s : samples)
		//		v.push_back(s.get_mean());
		//	return v;
		//}

		std::string to_string(bool full_sample = false) const {
			if (full_sample) {
				std::string s = "[";
				s += std::to_string(this->mean) + ", " + std::to_string(this->std) + ", " + std::to_string(this->num_samples) + "]";
				return s;
			} else {
				return std::to_string(this->mean);
			}
		}

		static std::vector<Sample> combine_samples(const std::vector<Sample>& samples1, const std::vector<Sample>& samples2) {
			if (samples1.size() != samples2.size())
				throw std::invalid_argument("Cannot combine samples; incongruent lentgh.");

			std::vector<Sample> samples(samples1.size());
			for (uint32_t i = 0; i < samples1.size(); i++)
				samples[i] = samples1[i].combine(samples2[i]);

			return samples;
		}

		static Sample collapse_samples(const std::vector<Sample>& samples) {
			size_t N = samples.size();
			if (N == 0)
				return Sample();

			Sample s = samples[0];
			for (uint32_t i = 1; i < samples.size(); i++)
				s = s.combine(samples[i]);

			return s;
		}

		static std::vector<Sample> collapse_samples(const std::vector<std::vector<Sample>>& samples) {
			size_t N = samples.size();
			if (N == 0)
				return std::vector<Sample>();

			size_t M = samples[0].size();

			std::vector<Sample> collapsed_samples(M);
			for (uint32_t i = 0; i < M; i++) {
				Sample s = samples[0][i];
				for (uint32_t j = 0; j < N; j++)
					s = s.combine(samples[j][i]);
			
				collapsed_samples[i] = s;
			}

			return collapsed_samples;
		}

    private:
        double mean;
        double std;
        uint32_t num_samples;
};


class data_t {
    private:
        std::map<std::string, std::vector<Sample>> data;
    
    public:
        data_t()=default;

        void emplace(const std::string &key, const std::vector<Sample> &samples) {
            data.emplace(key, samples);
        }

        void emplace(const std::string &key, const std::vector<double> &doubles) {
            size_t N = doubles.size();
            std::vector<Sample> samples(N);
            for (uint32_t i = 0; i < N; i++)
                samples[i] = Sample(doubles[i]);

            emplace(key, samples);
        }

        void emplace(const std::string &key, double d) {
            std::vector<Sample> sample{Sample(d)};
			emplace(key, sample);
        }

		void emplace(const std::string &key, Sample s) {
			std::vector<Sample> sample{s};
			emplace(key, sample);
		}

		auto begin() const {
			return data.begin();
		}

		auto end() const {
			return data.end();
		}
		
		int count(const std::string &key) const {
			return data.count(key);
		}

		const std::vector<Sample>& operator[](const std::string& key) const {
        	return data.at(key);
    	}

		std::vector<Sample>& operator[](const std::string& key) {
			return data[key];
		}

};

}