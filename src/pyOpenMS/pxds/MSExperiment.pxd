from libcpp.vector cimport vector as libcpp_vector
from MSSpectrum cimport *
from MSChromatogram cimport *
from DataValue cimport *
from String cimport *
from Peak1D cimport *
from ChromatogramPeak cimport *
from MetaInfoInterface cimport *
from ExperimentalSettings cimport *
from DateTime cimport *
from RangeManager cimport *

# this class has addons, see the ./addons folder

cdef extern from "<OpenMS/KERNEL/MSExperiment.h>" namespace "OpenMS":

    cdef cppclass MSExperiment(ExperimentalSettings, RangeManager2):
        # wrap-inherits:
        #   ExperimentalSettings
        #   RangeManager2

        MSExperiment() nogil except +
        MSExperiment(MSExperiment &)  nogil except +

        bool operator==(MSExperiment) nogil except +
        void reset() nogil except +
        bool clearMetaDataArrays() nogil except +
        ExperimentalSettings getExperimentalSettings() nogil except +
        
        void getPrimaryMSRunPath(StringList& toFill) nogil except +

        void swap(MSExperiment) nogil except +

        # Spectra functions
        void addSpectrum(MSSpectrum spec) nogil except +
        MSSpectrum operator[](int)      nogil except + # wrap-upper-limit:size()
        MSSpectrum getSpectrum(Size id_) nogil except +
        void setSpectra(libcpp_vector[ MSSpectrum ] & spectra) nogil except +
        libcpp_vector[MSSpectrum] getSpectra() nogil except +

        libcpp_vector[MSSpectrum].iterator begin() nogil except +        # wrap-iter-begin:__iter__(MSSpectrum)
        libcpp_vector[MSSpectrum].iterator end()    nogil except +       # wrap-iter-end:__iter__(MSSpectrum)

        # Chromatogram functions
        MSChromatogram getChromatogram(Size id_) nogil except +
        void addChromatogram(MSChromatogram chromatogram) nogil except +
        void setChromatograms(libcpp_vector[MSChromatogram] chromatograms) nogil except +
        libcpp_vector[MSChromatogram] getChromatograms() nogil except +

        MSChromatogram getTIC() nogil except +
        void clear(bool clear_meta_data) nogil except +

        void updateRanges() nogil except +
        void updateRanges(int msLevel) nogil except +

        void reserveSpaceSpectra(Size s) nogil except +
        void reserveSpaceChromatograms(Size s) nogil except +

        double getMinMZ() nogil except +
        double getMaxMZ() nogil except +
        double getMinRT() nogil except +
        double getMaxRT() nogil except +

        # Size of experiment
        UInt64 getSize() nogil except +
        int   size() nogil except +
        void resize(Size s) nogil except +
        bool empty() nogil except +
        void reserve(Size s) nogil except +
        Size getNrSpectra() nogil except +
        Size getNrChromatograms() nogil except +
        libcpp_vector[unsigned int] getMSLevels() nogil except +  # wrap-ignore

        void sortSpectra(bool sort_mz) nogil except +
        void sortSpectra() nogil except +
        void sortChromatograms(bool sort_rt) nogil except +
        void sortChromatograms() nogil except +

        bool isSorted(bool check_mz) nogil except +
        bool isSorted() nogil except +

        # from MetaInfoInterface:
        void getKeys(libcpp_vector[String] & keys) nogil except +
        void getKeys(libcpp_vector[unsigned int] & keys) nogil except + # wrap-as:getKeysAsIntegers
        DataValue getMetaValue(unsigned int) nogil except +
        DataValue getMetaValue(String) nogil except +
        void setMetaValue(unsigned int, DataValue) nogil except +
        void setMetaValue(String, DataValue) nogil except +
        bool metaValueExists(String) nogil except +
        bool metaValueExists(unsigned int) nogil except +
        void removeMetaValue(String) nogil except +
        void removeMetaValue(unsigned int) nogil except +

