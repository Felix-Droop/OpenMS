// --------------------------------------------------------------------------
//                   OpenMS -- Open-Source Mass Spectrometry
// --------------------------------------------------------------------------
// Copyright The OpenMS Team -- Eberhard Karls University Tuebingen,
// ETH Zurich, and Freie Universitaet Berlin 2002-2020.
//
// This software is released under a three-clause BSD license:
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of any author or any participating institution
//    may be used to endorse or promote products derived from this software
//    without specific prior written permission.
// For a full list of authors, refer to the file AUTHORS.
// --------------------------------------------------------------------------
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL ANY OF THE AUTHORS OR THE CONTRIBUTING
// INSTITUTIONS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// --------------------------------------------------------------------------
// $Maintainer: Hendrik Weisser $
// $Authors: Steffen Sass, Hendrik Weisser $
// --------------------------------------------------------------------------


#pragma once

#include <OpenMS/CONCEPT/Types.h>
#include <OpenMS/CHEMISTRY/AASequence.h>
#include <OpenMS/OpenMSConfig.h>
#include <OpenMS/config.h>

#include <boost/unordered_map.hpp>

#include <map> // for multimap<>
#include <vector> // for vector<>
#include <set> // for set<>
#include <utility> // for pair<>


namespace OpenMS
{
  class GridFeature;

  // Boost switch since with 1.47 several classes got moved into a new
  // boost::unordered namespace (specifically unordered_map).
  namespace OpenMSBoost
  {
#if OPENMS_BOOST_VERSION_MINOR > 47
    using namespace boost::unordered;
#else
    using namespace boost;
#endif
  }


/**
     @brief A representation of a QT cluster used for feature grouping.

     Ultimately, a cluster represents a group of corresponding features (or
     consensus features) from different input maps (feature maps or consensus
     maps).

     Clusters are defined by their center points (one feature each). A cluster
     also stores a number of potential cluster elements (other features) from
     different input maps, together with their distances to the cluster center.
     Every feature that satisfies certain constraints with respect to the
     cluster center is a @e potential cluster element. However, since a feature
     group can only contain one feature from each input map, only the "best"
     (i.e. closest to the cluster center) such feature is considered a true
     cluster element. To save memory, only the "best" element for each map is
     stored inside a cluster.

     The QT clustering algorithm has the characteristic of initially producing
     all possible, overlapping clusters. Iteratively, the best cluster is then
     extracted and the clustering is recomputed for the remaining points.

     In our implementation, multiple rounds of clustering are not necessary.
     Instead, the clustering is updated in each iteration. This is the reason
     for storing all potential cluster elements: When a certain cluster is
     finalized, its elements have to be removed from the remaining clusters,
     and affected clusters change their composition. (Note that clusters can
     also be invalidated by this, if the cluster center is being removed.)

     The quality of a cluster is the normalized average distance to the cluster
     center for present and missing cluster elements. The distance value for
     missing elements (if the cluster contains no feature from a certain input
     map) is the user-defined threshold that marks the maximum allowed radius
     of a cluster.

     When adding elements to the cluster, the client needs to call
     initializeCluster first and the client needs to call finalizeCluster after
     adding the last element.  After finalizeCluster, the client may not add
     any more elements through the add function (the client must call
     initializeCluster again before adding new elements).

     @see QTClusterFinder

     @ingroup Datastructures
*/
  class OPENMS_DLLAPI QTCluster
  {
private:

    // need to store more than one
    typedef std::multimap<double, GridFeature*> NeighborListType;
    typedef OpenMSBoost::unordered_map<Size, NeighborListType> NeighborMapMulti;

    typedef std::pair<double, GridFeature*> NeighborPairType;
    typedef OpenMSBoost::unordered_map<Size, NeighborPairType> NeighborMap;

    /// Pointer to the cluster center
    GridFeature* center_point_;

    /**
     * @brief Map that keeps track of the best current feature for each map
     *
     */
    NeighborMap neighbors_;

    /**
     * @brief Temporary map tracking *all* neighbors
     *
     * For each input run, a multimap which contains pointers to all
     * neighboring elements and the respective distance.
     *
     */
    NeighborMapMulti* tmp_neighbors_;

    /// Maximum distance of a point that can still belong to the cluster
    double max_distance_;

    /// Number of input maps
    Size num_maps_;

    /// Quality of the cluster
    double quality_;

    /// Has the cluster changed (if yes, quality needs to be recomputed)?
    bool changed_;

    /// Keep track of peptide IDs and use them for matching?
    bool use_IDs_;

    /// Whether current cluster is valid
    bool valid_;

    /** 
     * @brief Whether initial collection of all neighbors is needed
     *
     * This variable stores whether we need to collect all annotations first
     * before we can decide upon the best set of cluster points. This is
     * usually only necessary if the center point does not have an annotation
     * but we want to use ids.
     *
    */
    bool collect_annotations_;

    /// Whether current cluster is accepting new elements or not (if true, no more new elements allowed)
    bool finalized_;

    /// x coordinate in the grid cell
    Int x_coord_;

    /// y coordinate in the grid cell
    Int y_coord_;

    /**
     * @brief Set of annotations of the cluster
     *
     * The set of peptide sequences that is compatible to the cluster center
     * and results in the best cluster quality.
     */
    std::set<AASequence> annotations_;

    /// Base constructor (not accessible)
    QTCluster();

    /// Computes the quality of the cluster
    void computeQuality_();

    /**
     * @brief Finds the optimal annotation (peptide sequences) for the cluster
     *
     * The optimal annotation is the one that results in the best quality. It
     * is stored in @p annotations_;
     *
     * This function is only needed when peptide ids are used and the current
     * center point does not have any peptide id associated with it. In this
     * case, it is not clear which peptide id the current cluster should use.
     * The function thus iterates through all possible peptide ids and selects
     * the one producing the best cluster.
     *
     * This function needs access to all possible neighbors for this cluster
     * and thus can only be run when tmp_neighbors_ is filled (which is during
     * the filling of a cluster). The function thus cannot be called after
     * finalizing the cluster.
     *
     * @returns The total distance between cluster elements and the center.
     */
    double optimizeAnnotations_();

public:

    /**
     * @brief Detailed constructor
     * @param center_point Pointer to the center point
     * @param num_maps Number of input maps
     * @param max_distance Maximum allowed distance of two points
     * @param use_IDs Use peptide annotations?
     */
    QTCluster(GridFeature* center_point, Size num_maps,
              double max_distance, bool use_IDs, 
              Int x_coord, Int y_coord);

    /// Destructor
    ~QTCluster();

    /// Returns the cluster center
    GridFeature* getCenterPoint();

    /// Returns the RT value of the cluster
    double getCenterRT() const;

    /// Returns the m/z value of the cluster center
    double getCenterMZ() const;

    /// Returns the x coordinate in the grid
    Int getXCoord() const;

    /// Returns the y coordinate in the grid
    Int getYCoord() const;

    /// Returns the size of the cluster (number of elements, incl. center)
    Size size() const;

    /// Compare by quality
    bool operator<(QTCluster& cluster);

    /**
     * @brief Adds a new element/neighbor to the cluster
     * @note There is no check whether the element/neighbor already exists in the cluster!
     * @param element The element to be added
     * @param distance Distance of the element to the center point
     */
    void add(GridFeature* element, double distance);

    /// Gets the clustered elements
    void getElements(OpenMSBoost::unordered_map<Size, GridFeature*>& elements);

    /**
     * @brief Updates the cluster after the indicated data points are removed
     *
     * @param removed The datapoints to be removed from the cluster
     *
     * @return Whether the cluster composition has changed due to the update
     */
    bool update(const OpenMSBoost::unordered_map<Size, GridFeature*>& removed);

    /// Returns the cluster quality
    double getQuality();

    /// Return the set of peptide sequences annotated to the cluster center
    const std::set<AASequence>& getAnnotations();

    /**
     * @brief Sets current cluster as invalid (also frees some memory)
     *
     * @note Do not attempt to use the cluster again once it is invalid, some
     * internal data structures have now been cleared
     *
     */
    void setInvalid();

    /// Whether current cluster is invalid
    inline bool isInvalid() const
    {
      return !valid_;
    }

    /// Has to be called before adding elements (calling QTCluster::add)
    void initializeCluster();

    /// Has to be called after adding elements (after calling QTCluster::add one or multiple times)
    void finalizeCluster();

    /// Get all current neighbors
    OpenMSBoost::unordered_map<Size, std::vector<GridFeature*> > getAllNeighbors();

    NeighborMap const& getAllNeighborsDirect();

    void makeSeq_table(std::map<std::set<AASequence>, std::vector<double>> &seq_table) const;

    void initialize_neighbors_();
  };
} // namespace OpenMS

